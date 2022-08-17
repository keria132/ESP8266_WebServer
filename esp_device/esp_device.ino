/*Code for ESP-01, which transfers data from DHT sensor to the main server via WiFi connection.*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHT.h"
int id = 0; //to identify from which esp data will be received

// Set your Static IP address
IPAddress local_IP(192, 168, 0, 101);
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
const char *ssid = "ssid";
const char *password = "password";

ESP8266WebServer server(80);

void setup() {

  delay(1000);
  Serial.begin(115200);

  pinMode(lightsPin, OUTPUT);
  
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
//  
  Temperature = dht.readTemperature(); // Get temperature from sensor
  Humidity = dht.readHumidity();       // Get humidity from sensor
  
  delay(3000);

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

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  char c = client.read();
  Serial.print(c);
  while (client.available() == 0) {
    if (millis() - timeout > 2000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
}
