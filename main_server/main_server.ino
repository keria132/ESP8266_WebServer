/*
  Main web-server for ESP8266 data monitoring
  You need to set your wifi ssid & password and DHT sensor pin
  1 device = 1 room
*/

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Set your Static IP address
IPAddress local_IP(192, 168, 0, 94);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// Replace with your network credentials
const char* ssid = "ssid";
const char* password = "password";

//DHT sensor settings
#define DHTTYPE DHT22   //Type - DHT 22  (AM2302), AM2321
uint8_t DHTPin = 13;    //ESP D7 pin
DHT dht(DHTPin, DHTTYPE);
float temperature, humidity;
bool dhtStatus;

unsigned long timer;
String responseJson;  //JSON object with data for response to the client
int dataArray[5][5];  //Array witch connected rooms data

// Create AsyncWebServer object on port 80
DynamicJsonBuffer jsonBuffer;
AsyncWebServer server(80);

//Make a response json data
void makeResponseJson(){

  responseJson = "";
  
  //Check if data is valid
  if(isnan(temperature) || isnan(humidity)){
    Serial.println("Incorrect data");
    temperature = 0;
    humidity = 0;
    dhtStatus = -1; //Change status of the sensor to negative
  }else{
    dhtStatus = 1;
  }
  
  //Add data from this main room/server
  StaticJsonBuffer<800> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& main_room = root.createNestedArray("main_room");
  main_room.add(temperature);
  main_room.add(humidity);
  main_room.add(dhtStatus);

  //Add data from other rooms/devices to array
  for(int i = 0; i <= 4; i++){
    //If device is ON
    if(dataArray[i][4] == 1){
      String roomName = "room" + String(i);
      JsonArray& room = root.createNestedArray(roomName);
      room.add(dataArray[i][0]);
      room.add(dataArray[i][1]);
      room.add(dataArray[i][2]);
      room.add(dataArray[i][3]);
    }
  }

  root.prettyPrintTo(responseJson);
}

//Send json object on /getData request
String getData(){
  makeResponseJson();
  //Serial.println("Complete json:");
  //Serial.println(responseJson);
  return String(responseJson);
}

//Add or update other rooms data via request from devices
void handle_receiveData(AsyncWebServerRequest *request){
   //Receive variables from other esp
  String sensorValues, deviceName = "room";

  if (request->hasParam("sensor_reading")){
    sensorValues = request->arg("sensor_reading");
  }

  //Parse data as json object
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(sensorValues);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }else{
    deviceName += String(root["esp_num"]);
  }

  int temp, hum, id;
  temp = root["dhtTemp_reading"];
  hum = root["dhtHum_reading"];
  id = root["esp_num"];
  
  //Check if data is valid
  if(isnan(temp) || isnan(hum)){
    Serial.println("Incorrect incoming data");
    temp = 0;
    hum = 0;
    dataArray[id][3] = -1;//Change time to negative to let know client that data corrupted
    dataArray[id][4] = -1;
  }else{
    dataArray[id][0] = temp;
    dataArray[id][1] = hum;
    dataArray[id][2] = 0; //Time since last connection
    dataArray[id][3] = id;//Device id
    dataArray[id][4] = 1; //State of the device ON/OFF/DATA ERROR
  }
  
}
 
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(DHTPin, INPUT);
  dht.begin();

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Client HTTP request to update data
  server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getData().c_str());
  });

  // ESP device HTTP request to receive data
  server.on("/receiveData", HTTP_GET, handle_receiveData);

  // Start server
  server.begin();
}

void loop(){

  temperature = dht.readTemperature(); // Get temperature
  humidity = dht.readHumidity();       // Get humidity

  //Add seconds since the last device connection
  if(millis()-timer >= 1000){
    for(int i = 0; i <= 5; i++){
      if( dataArray[i][4] != 0){
        dataArray[i][2] += 1;
      }
    }
    timer = millis();
  }
  
}
