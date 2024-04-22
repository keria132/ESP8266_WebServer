#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Set your Static IP address
IPAddress local_IP(192, 168, 0, 100);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

//SSID and Password to connect to wifi
const char *ssid = "Tobik_Hata";
const char *password = "P4npYfYS";

AsyncWebServer server(80);

//JSON that stores relays/devices data
StaticJsonDocument<400> relaysJSON;

void setup() {

  Serial.begin(115200);

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
  delay(1000);

 // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false);
  });

  // Route to load style.css file
  server.on("/style.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.min.css", "text/css");
  });

  // Route to load style.css file
  server.on("/script.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.min.js", "text/javascript");
  });

  server.on("/lights__icon.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/lights__icon.svg");
  });

  //Send weather data from weather API
  server.on("/updateRelays", HTTP_GET, handle_getWeatherData);

  //Relays data request from server webpage
  server.on("/getRelaysData", HTTP_GET, handle_getRelaysData);

  //Receive relay data from another ESP device
  server.on("/updateRelays", HTTP_GET, handle_updateRelays);

  server.begin();
}

void loop() {
}

void handle_updateRelays(AsyncWebServerRequest *request){

  Serial.print("Hadling device request...");

  // String relayData = "";

  // if (request->hasParam("relayData")){
  //   relayData = request->arg("relayData");
  // }
  
  // Serial.print("Received value:");
  // Serial.println(relayData);

  // //Pull IP adress from received device with JSON
  // DynamicJsonDocument receivedRelay(1024);
  // deserializeJson(receivedRelay, relayData);
  // String relayIP = receivedRelay["relay_ip"];

  // //Find out if we alreadyhave device with this IP adress
  // const char* hasRelay = relaysJSON[{}]["relay_ip"];
  
  // if (String(hasRelay) == relayIP) {
  //   Serial.println("Already contains relay with ip: " + String(hasRelay) + ", returning...");
  //   return;
  // }

  // //Put new received relay data in JSON object
  // JsonObject relay  = relaysJSON.createNestedObject();
  // relay["relay_ip"] = receivedRelay["relay_ip"];
  // relay["relay_name"] = receivedRelay["relay_name"];
  // relay["relay_status"] = receivedRelay["relay_status"];
  // //Serial.print("JSON object: ");
  // serializeJson(relaysJSON, Serial);
  // Serial.println();

  // request->send(200, "text/plain", "ok");  
}

void handle_getRelaysData(AsyncWebServerRequest *request){

  Serial.println("Handling webpage data request...");
  // serializeJson(relaysJSON, Serial);
  // Serial.println();
  // String receivedData = "";
  // serializeJson(relaysJSON, receivedData);
  
  // request->send(200, "text/plain", receivedData);
}

void handle_getWeatherData(AsyncWebServerRequest *request){
  
}
