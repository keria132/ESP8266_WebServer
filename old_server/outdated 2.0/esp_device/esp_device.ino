/*Code for ESP-01, which transfers data from DHT sensor to the main server via WiFi connection.*/

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "DHT.h"
int id = 0; //to identify from which esp data will be received
int ipId = 100 + id + 1;
short firstRelayPin = 5; //GPIO5 = D1 PIN
short secondRelayPin = 4;  //GPI04 = D2 PIN
unsigned long timer;

// Set your Static IP address
IPAddress local_IP(192, 168, 0, ipId);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

//DHT sensor
#define DHTTYPE DHT22   // Define dht type - DHT 22
uint8_t DHTPin = 13; //D7 pin
DHT dht(DHTPin, DHTTYPE);
float Temperature;
float Humidity;

//SSID and Password to connect to wifi
const char *ssid = "Tobik_Hata";
const char *password = "P4npYfYS";

AsyncWebServer server(80);

void setup() {

  delay(1000);
  Serial.begin(115200);

  pinMode(firstRelayPin, OUTPUT);
  pinMode(secondRelayPin, OUTPUT);
  
  pinMode(DHTPin, INPUT);
  dht.begin();

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");

  server.on("/relaySwitch/", HTTP_GET, handle_relaySwitch);
  server.begin();
}

void loop() {

  if(millis()-timer >= 3000){
    sendSensorData();
    timer = millis();
  }
  
}

void sendSensorData(){

  Temperature = dht.readTemperature(); // Get temperature from sensor
  Humidity = dht.readHumidity();       // Get humidity from sensor
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const char * host = "192.168.0.94"; //Main esp server ip
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection to host failed");
    return;
  }else{
    Serial.println("Connection established");
  }

  // We now create a URL for the request. Something like /data/?sensor_reading=123
  String url = "/receiveData/";
  url += "?sensor_reading=";
  url +=  "{\"dhtTemp_reading\":\"dhtTemp_value\",\"dhtHum_reading\":\"dhtHum_value\",\"esp_num\":\"id\"}";
  url.replace("dhtTemp_value", String(Temperature));
  url.replace("dhtHum_value", String(Humidity));
  url.replace("id", String(id));
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "Connection: close\r\n\r\n");
  client.stop();
}

void handle_relaySwitch(AsyncWebServerRequest *request){
  String receivedValue;

  if (request->hasParam("button_reading")){
    receivedValue = request->arg("button_reading");
  }
  Serial.println(receivedValue);

  //Yes, bunch of "if", i was too lazy to think about better solution
  if(receivedValue == "00"){
    digitalWrite(firstRelayPin, 0);
  }
  if(receivedValue == "01"){
    digitalWrite(firstRelayPin, 1);
  }
  if(receivedValue == "10"){
    digitalWrite(secondRelayPin, 0);
  }
  if(receivedValue == "11"){
    digitalWrite(secondRelayPin, 1);
  }
  
  receivedValue += "\e";
  
  request->send(200, "text/plain", receivedValue);
  
}
