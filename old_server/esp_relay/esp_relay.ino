/*ESP which controls relay via main server request*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
int lightsPin = 5;
String lightsStatus;
unsigned long wifi_status_timer;

// Set your Static IP address
IPAddress local_IP(192, 168, 0, 101);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

//SSID and Password to connect to wifi
const char *ssid = "ssid";
const char *password = "password";

ESP8266WebServer server(80);

void setup() {

  delay(1000);
  Serial.begin(115200);
  
  pinMode(lightsPin, OUTPUT);

// Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
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
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  //Reconnect if wifi goes off and appears after
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  server.on("/lightsChange/", HTTP_GET, handle_lightsChange);
  server.begin();
}

void loop() {
  server.handleClient();

  //Check the Wi-Fi status every 30 seconds
    switch (WiFi.status()){
      case WL_NO_SSID_AVAIL:
        Serial.println("Configured SSID cannot be reached");
        break;
      case WL_CONNECTED:
        Serial.println("Connection successfully established");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("Connection failed");
        Serial.println("Trying to reconnect");
        WiFi.disconnect();
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) 
        {
          delay(1000);
          Serial.print(".");
        }
        break;
    }
    Serial.printf("Connection status: %d\n", WiFi.status());
    Serial.print("RRSI: ");
    Serial.println(WiFi.RSSI());
    wifi_status_timer = millis()/1000;
  }
  
}

void handle_lightsChange() {
  if (server.hasArg("button_reading"))
  {
    lightsStatus = server.arg("button_reading");
  }
  if(lightsStatus == "0"){
    Serial.println(lightsStatus);
    digitalWrite(lightsPin, 1);
  }else if (lightsStatus == "1"){
    Serial.println(lightsStatus);
    digitalWrite(lightsPin, 0);
  }
  
}
