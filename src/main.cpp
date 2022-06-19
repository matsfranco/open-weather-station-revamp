#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <SFE_BMP180.h>
#include <BH1750.h>
#define I2C_SCL 12
#define I2C_SDA 13
#define ACTIVITY_LED 15 // D8
#define BUZZER 2 // D4
bool sensorStatus[5] = {false,false,false,false,false};

// DHT22
#define DHT22_INDEX 0
#define DHTPIN 14     // D5
#define DHTTYPE DHT22

// WIFI Spot
const char* ssid = "SSID";
const char* password = "PSWD";
String apiKey = "API_KEY";
const char* serverName = "http://api.thingspeak.com/update";
unsigned long timerDelay = 10000;
#define CONN_STATUS_LED 13 // D7

// BMP180
#define BMP180_INDEX 1
SFE_BMP180 bmpSensor;
double bmpTemperature, bmpPressure;

// BH1750
#define BH1750_INDEX 2
BH1750 lightMeter;
float lux;

// LM393 + YL-38
#define YL38_INDEX 3
#define YL38_PIN 12 // D6
bool isRaining = false;

DHT dht(DHTPIN, DHTTYPE);
float dhtTemperature;
float dhtHumidity;
float dhtHeatIndex;

int timeSinceLastRead = 0;

void connectToRouter() {
  digitalWrite(CONN_STATUS_LED,LOW);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  bool status = false;
  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    status = !status;
    digitalWrite(CONN_STATUS_LED,status);
  }
  digitalWrite(CONN_STATUS_LED,HIGH);
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void sendDataToServer() {
  if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String httpRequestData = "api_key="+apiKey;
      if(sensorStatus[0]) httpRequestData+="&field1="+String(dhtTemperature);
      if(sensorStatus[0]) httpRequestData+="&field2="+String(dhtHeatIndex);
      if(sensorStatus[0]) httpRequestData+="&field3="+String(dhtHumidity);
      if(sensorStatus[1]) httpRequestData+="&field4="+String(bmpPressure);
      if(sensorStatus[1]) httpRequestData+="&field5="+String(bmpTemperature);
      if(sensorStatus[2]) httpRequestData+="&field6="+String(lux);
      if(sensorStatus[3]) httpRequestData+="&field7="+String(isRaining);
      int httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
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

void yl_getData() {
  if(digitalRead(YL38_PIN)) {
    isRaining = 0;
  } else {
    isRaining = 1;
  }
  Serial.print("isRaining: ");
  Serial.println(isRaining);
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
  pinMode(ACTIVITY_LED,OUTPUT);
  digitalWrite(ACTIVITY_LED,LOW);
  pinMode(CONN_STATUS_LED,OUTPUT);
  pinMode(BUZZER,OUTPUT);
  digitalWrite(BUZZER,LOW);
  pinMode(YL38_PIN,INPUT);

  connectToRouter();
  Serial.setTimeout(2000);

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
    Serial.println("BH1750 Running!! CONTINUOUS_HIGH_RES_MODE");
    Serial.println("-------------------------------------");  
  } else {
    sensorStatus[2] = false;
    Serial.println(F("BH1750 INIT FAIL"));
    Serial.println("-------------------------------------"); 
  }

  Serial.println("YL-38 Started");
  Serial.println("-------------------------------------");
  sensorStatus[3] = true;   
  Serial.println("YL-38 Running!!");
  Serial.println("-------------------------------------");
  sensorStatus[0] = true;
  sensorStatus[1] = false;
  sensorStatus[2] = false;
  sensorStatus[3] = false;

}

void loop() {
  if(timeSinceLastRead > 60000) {
    digitalWrite(ACTIVITY_LED,HIGH);
    if(sensorStatus[DHT22_INDEX]) {
      tone(BUZZER,440);
      Serial.println(">> DHT22 Data");
      dht_getData();
      printSensorData(dhtTemperature,dhtHumidity,dhtHeatIndex);
      noTone(BUZZER);
    } 
    if(sensorStatus[BMP180_INDEX]) {
      tone(BUZZER,440);
      Serial.println(">> BMP180 Data");
      bmp_getData();
      noTone(BUZZER);
    }
    if(sensorStatus[BH1750_INDEX]) {
      tone(BUZZER,440);
      Serial.println(">> BH1750 Data");
      bh_getData();
      noTone(BUZZER);
    }
    if(sensorStatus[YL38_INDEX]) {
      tone(BUZZER,440);
      Serial.println(">> YL-38 Data");
      yl_getData();
      noTone(BUZZER);
    }

    tone(BUZZER,440);
    sendDataToServer();
    timeSinceLastRead = 0;
    digitalWrite(ACTIVITY_LED,LOW);
    noTone(BUZZER);
  }
  delay(100);
  timeSinceLastRead += 100;
}