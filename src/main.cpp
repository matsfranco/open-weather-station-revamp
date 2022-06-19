#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <SFE_BMP180.h>
#include <BH1750.h>

// DHT22
#define DHTPIN 14     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

#define I2C_SCL 12
#define I2C_SDA 13
float dst,bt,bp,ba;
char dstmp[20],btmp[20],bprs[20],balt[20];
bool bmp085_present=true;


bool sensorStatus[5] = {false,false,false,false,false};
#define DHT22_INDEX 0
#define BMP180_INDEX 1
#define BH1750_INDEX 2

// WIFI Spot
const char* ssid = "SSID";
const char* password = "PSWD";
unsigned long timerDelay = 10000;

// BMP180
SFE_BMP180 bmpSensor;
double bmpTemperature, bmpPressure;

//BH1750
BH1750 lightMeter;
float lux;

// Thingspeak
// Domain Name with full URL Path for HTTP POST Request
const char* serverName = "http://api.thingspeak.com/update";
// Service API Key
String apiKey = "API_KEY";


DHT dht(DHTPIN, DHTTYPE);
float dhtTemperature;
float dhtHumidity;
float dhtHeatIndex;

int timeSinceLastRead = 0;

void connectToRouter() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  // Random seed is a number used to initialize a pseudorandom number generator
  randomSeed(analogRead(0));
}

void sendDataToServer() {
  if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "api_key="+apiKey;
      httpRequestData+="&field1="+String(dhtTemperature);
      httpRequestData+="&field2="+String(dhtHeatIndex);
      httpRequestData+="&field3="+String(dhtHumidity);
      httpRequestData+="&field4="+String(bmpPressure);
      httpRequestData+="&field5="+String(bmpTemperature);
      httpRequestData+="&field6="+String(lux);

      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
           
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}

float getTemperature() {
  return dht.readTemperature();
}

float getHumidity() {
  return dht.readHumidity();
}

float getHeatIndex(float temperature, float humidity) {
  return dht.computeHeatIndex(temperature, humidity, false);
}

void dht_getData() {
  dhtTemperature = getTemperature();
  dhtHumidity = getHumidity();
  if (isnan(dhtHumidity) || isnan(dhtTemperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    dhtHeatIndex = getHeatIndex(dhtTemperature,dhtHumidity);
  }
}

void bmp_getData() {
  char status;
  status = bmpSensor.startTemperature();
  if (status != 0) {
    delay(status);
    status = bmpSensor.getTemperature(bmpTemperature);
    if(status != 0) {
      Serial.print("BMP180_temperature: ");
      Serial.print(bmpTemperature,2);
      Serial.println(" deg C");
      status = bmpSensor.startPressure(3);
      if(status != 0) {
        delay(status);
        status = bmpSensor.getPressure(bmpPressure,bmpTemperature);
        if(status != 0) {
          Serial.print("BMP180_abs_pressure: ");
          Serial.print(bmpPressure,2);
          Serial.println(" mb");
        }
      }
    }
  }
}

void bh_getData() {
  if (lightMeter.measurementReady()) {  
    lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
  }
}


void printSensorData(float temperature, float humidity, float heatIndex) {
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C ");
  Serial.print("Heat index: ");
  Serial.print(heatIndex);
  Serial.println(" *C ");
}

void setup() {
  Serial.begin(9600);
  connectToRouter();
  Serial.setTimeout(2000);
  // Wait for serial to initialize.
  while(!Serial) { }
  Wire.begin(D2, D1);
  dht.begin();

  Serial.println("DHT22 Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!!");
  Serial.println("-------------------------------------");
  sensorStatus[0] = true;

  Serial.println("BMP180 Started");
  Serial.println("-------------------------------------");  
  if (bmpSensor.begin()) {
    sensorStatus[1] = true;
    Serial.println("Running BMP180!!");
    Serial.println("-------------------------------------");  
  } else {
    Serial.println("BMP180 INIT FAIL\n\n");
    Serial.println("-------------------------------------"); 
    sensorStatus[1] = false;
  }

  Serial.println("BH1750 Started");
  Serial.println("-------------------------------------");
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    sensorStatus[2] = true;
    Serial.println("BH1750 Running in mode CONTINUOUS_HIGH_RES_MODE");
    Serial.println("-------------------------------------");  
  } else {
    sensorStatus[2] = false;
    Serial.println(F("BH1750 INIT FAIL"));
    Serial.println("-------------------------------------"); 
  }
}

void loop() {
  if(timeSinceLastRead > 30000) {
    if(sensorStatus[DHT22_INDEX]) {
      Serial.println(">> DHT22 Data");
      dht_getData();
      printSensorData(dhtTemperature,dhtHumidity,dhtHeatIndex);
    } 
    if(sensorStatus[BMP180_INDEX]) {
      Serial.println(">> BMP180 Data");
      bmp_getData();
    }
    if(sensorStatus[BH1750_INDEX]) {
      Serial.println(">> BH1750 Data");
      bh_getData();
    }

    sendDataToServer();

    timeSinceLastRead = 0;
  }
  delay(100);
  timeSinceLastRead += 100;
}