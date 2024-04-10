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
const char* ssid = "Tobik_Hata";
const char* password = "P4npYfYS";

//DHT sensor settings
#define DHTTYPE DHT22   //Type - DHT 22  (AM2302), AM2321
uint8_t DHTPin = 13;    //ESP D7 pin
DHT dht(DHTPin, DHTTYPE);
float temperature, humidity;
bool dhtStatus;

unsigned long timer;
String responseJson;  //JSON object with data for response to the client
int roomsData[5][7];  //Array which contains rooms data
bool relayStatus[2];

bool functionCall = false;
bool currentRelayState[2];
char currentHost[13] = "192.168.0.10";


// Create AsyncWebServer object on port 80
DynamicJsonBuffer jsonBuffer;
AsyncWebServer server(80);
 
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

  // Send data to the client
  server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getData().c_str());
  });

  // Receive data from other ESP device
  server.on("/receiveData", HTTP_GET, handle_receiveData);

  // Switch checkbox
  server.on("/checkboxSwitch", HTTP_GET, handle_checkboxSwitch);

  // Start server
  server.begin();
}

void loop(){

  temperature = dht.readTemperature(); // Get temperature
  humidity = dht.readHumidity();       // Get humidity

  //Add seconds since the last device connection
  if(millis()-timer >= 1000){
    for(int i = 0; i <= 5; i++){
      if( roomsData[i][6] != 0){
        roomsData[i][2] += 1;
      }
    }
    //relaySwitch();
    timer = millis();
  }
  
  if(functionCall == true){
    relaySwitch();
    functionCall = false;
  }
  
}

//Send json object on /getData request
String getData(){
  makeResponseJson();
  //Serial.println("Complete json:");
  //Serial.println(responseJson);
  return String(responseJson);
}

//Make response json object
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
  main_room.add(""); //add the fuckin id for the love of god
  main_room.add(relayStatus[0]);
  main_room.add(relayStatus[1]);

  //Add data from other rooms/devices to array
  for(int i = 0; i <= 4; i++){
    //If device is ON
    if(roomsData[i][6] == 1){
      String roomName = "room" + String(i);
      JsonArray& room = root.createNestedArray(roomName);
      room.add(roomsData[i][0]); //Temperature
      room.add(roomsData[i][1]); //Humidity
      room.add(roomsData[i][2]); //Time since last receive
      room.add(roomsData[i][3]); //id
      room.add(roomsData[i][4]);
      room.add(roomsData[i][5]);
    }
  }
  root.prettyPrintTo(responseJson);
}



//Add or update other rooms data via request from devices
void handle_receiveData(AsyncWebServerRequest *request){
  
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
    temp = 100;
    hum = 0;
    roomsData[id][3] = -1;//Change time to negative to let know client that data corrupted
    roomsData[id][4] = -1;
  }else{
    roomsData[id][0] = temp;
    roomsData[id][1] = hum;
    roomsData[id][2] = 0; //Time since last connection
    roomsData[id][3] = id;//Device id
    roomsData[id][6] = 1; //State of the device ON/OFF/DATA ERROR
  }
  
}

void handle_checkboxSwitch(AsyncWebServerRequest *request){
  
  int roomNum = (request->arg("roomNum")).toInt();
  int checkboxNum = (request->arg("checkboxNum")).toInt();

  if(roomNum==0){
    relayStatus[checkboxNum] = !relayStatus[checkboxNum];
    
    request->send(200, "text/plain", String(relayStatus[checkboxNum]));
  }else{
    roomsData[roomNum-1][checkboxNum+4] = !roomsData[roomNum-1][checkboxNum+4];

    request->send(200, "text/plain", String(roomsData[roomNum-1][checkboxNum+4]));

    currentRelayState[0] = checkboxNum;
    currentRelayState[1] = roomsData[roomNum-1][checkboxNum+4];

    
    char roomIp = roomNum + 48;
    currentHost[12] = roomIp;
    functionCall = true;
    
  }

}

//Connect to esp device to change relay state
void relaySwitch(){

  Serial.print("Relay state: ");
  Serial.println(currentRelayState[0]);
  WiFiClient client;
  const char * host = currentHost; //Relay controlling esp ip adress
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.print("Connection to host ");
    Serial.print(host);
    Serial.println(" failed");
    return;
  }else{
    Serial.print(host);
    Serial.println(" Connection established");
  }
  // We now create a URI for the request. Something like /data/?sensor_reading=123
  String url = "/relaySwitch/";
  url += "?button_reading=";
  url += currentRelayState[0];
  url += currentRelayState[1];

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "Connection: close\r\n\r\n");
  
//  String answer = client.readStringUntil('\e');
//  answer = answer.charAt(answer.length() - 1);
//  Serial.println(answer);
  
}
