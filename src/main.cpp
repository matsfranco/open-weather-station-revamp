#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define DHTPIN 14     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors


// WIFI Spot
const char* ssid = "SSID";
const char* password = "PSWD";
unsigned long timerDelay = 10000;

// Thingspeak
// Domain Name with full URL Path for HTTP POST Request
const char* serverName = "http://api.thingspeak.com/update";
// Service API Key
String apiKey = "API_KEY";


DHT dht(DHTPIN, DHTTYPE);

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

void sendDataToServer(float temperature, float heatIndex, float humidity) {
  if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "api_key="+apiKey+"&field1="+String(temperature)+"&field2="+String(heatIndex)+"&field3="+String(humidity);           
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
  
  dht.begin();

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");

}

int timeSinceLastRead = 0;

void loop() {

  // Report every 2 seconds.
  if(timeSinceLastRead > 30000) {
    float h = getHumidity();
    float t = getTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
    }
    float hic = getHeatIndex(t,h);
    printSensorData(t,h,hic);

    sendDataToServer(t,hic,h);

    timeSinceLastRead = 0;
  }
  delay(100);
  timeSinceLastRead += 100;
}