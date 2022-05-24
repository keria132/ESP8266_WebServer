/*Code for ESP-01, which transfers data from DHT sensor to the main server via WiFi connection.*/

#include <ESP8266WiFi.h>
#include "DHT.h"
unsigned long timer, resetTimer; //timer to send data once in some time interval
int id = 2;

//DHT sensor
#define DHTTYPE DHT22   // Define dht type - DHT 22
uint8_t DHTPin = 0; //GPIO 0 pin
DHT dht(DHTPin, DHTTYPE);
float Temperature;
float Humidity;

//SSID and Password to connect to wifi
const char *ssid = "Tobik_Hata";
const char *password = "P4npYfYS";


void setup() {

  delay(1000);
  Serial.begin(115200);
  
  pinMode(DHTPin, INPUT);
  dht.begin();

  // set the ESP8266 to be a WiFi-client
  WiFi.mode(WIFI_STA);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");

}

void loop() {
  
  Temperature = dht.readTemperature(); // Get temperature from sensor
  Humidity = dht.readHumidity();       // Get humidity from sensor

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const char * host = "192.168.0.94"; //Main esp server ip
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection to host failed");
    return;
  }

  // We now create a URI for the request. Something like /data/?sensor_reading=123
  String url = "/sensors_data/";
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
  while (client.available() == 0) {
    if (millis() - timeout > 2000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
}
