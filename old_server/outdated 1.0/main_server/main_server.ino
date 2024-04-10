/*Code for the main server, which will receive requests from other ESP8266 devices which are connected to the same WIFI spot
*/

//Libraries to work with wi-fi, server, etc...
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "DHT.h"

// Set your Static IP address
IPAddress local_IP(192, 168, 0, 94);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

//DHT sensor settings
#define DHTTYPE DHT22   //Type - DHT 22  (AM2302), AM2321
uint8_t DHTPin = 13;    //ESP D7 pin
DHT dht(DHTPin, DHTTYPE);
float Temperature, Humidity;

//WIFI ssid, password
const char* ssid = "Tobik_Hata";
const char* password = "P4npYfYS";

//Variables for incoming data from other ESP controllers
float outsideTemp, outsideHum, upstairsTemp, upstairsHum;
int outsideState = 0, upstairsState = 0;
int outsideUpdate, upstairsUpdate;
String sensor_values;
bool lightsStatus = 0;
unsigned long timer, wifi_status_timer;

DynamicJsonBuffer jsonBuffer;
ESP8266WebServer server(80);

void setup() {
  delay(1000);
  Serial.begin(115200);
  pinMode(DHTPin, INPUT);
  dht.begin();
  
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  //Setting wifi to STA mode
  WiFi.mode(WIFI_STA);
  
  //Setting STA mode
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

  //Reconnect if wifi goes off and appears after
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  //Server responses
  server.onNotFound(handle_NotFound);
  server.on("/", handle_OnConnect);
  server.on("/update_data", handle_update_data); //Function to update data
  server.on("/lightsSwitch", handle_lightsSwitch); //Function to send request to switch relay to other esp
  server.on("/sensors_data/", HTTP_GET, handleSentVar); // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  //Update timer for other esp(last time when informaion was received)
  if(millis()-timer >= 1000){
    outsideUpdate++;
    upstairsUpdate++;
    timer = millis();
  }

  //Check the Wi-Fi status every 30 seconds
  if (millis() - wifi_status_timer >=30000){
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
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
    wifi_status_timer = millis();
  }
  
}

//Not found page:
void handle_NotFound(){
  String html ="<!DOCTYPE html> <html id=\"webpage\"> <head> <meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\"> <title>Page not found</title> <style>@import url('https://fonts.googleapis.com/css2?family=Gemunu+Libre:wght@300;500;600&display=swap'); html { font-size: 4vw; font-family: 'Gemunu Libre', sans-serif; } body{ background-color: #222; color: white; } body main{ display: flex; flex-direction: column; justify-content: center; align-items: center; } body main h1{ font-size: 20vw; margin: 5vw 0 2vw 0; } body main h2{ margin: 2vw 0 0 0; } </style> </head> <body> <main> <h1> 404 </h1> <h2> Page not found </h2> </main> </body> </html>";
  server.send(404, "text/html", html);
}

//Main page:
void handle_OnConnect() {
  Temperature = dht.readTemperature(); // Get temperature
  Humidity = dht.readHumidity();       // Get humidity
  server.send(200, "text/html", SendHTML());
}

//Update data function
void handle_update_data(){
  Temperature = dht.readTemperature(); // Get temperature
  Humidity = dht.readHumidity();       // Get humidity

  //State of other esp devices: state 0 - not responding, state 1 is ok, state 2 - data is corrupted
  //Reset update timer and state if other esp-s don't send any data in 60 seconds
  if(outsideUpdate >=60){
    outsideState = 0;
    outsideTemp = 0;
    outsideHum = 0;
  }

  if(upstairsUpdate >=60){
    upstairsState = 0;
    upstairsTemp = 0;
    upstairsHum = 0;
  }

  //Check if data is valid
  if(isnan(Temperature) || isnan(Humidity)){
    Serial.println("Incorrect data");
    Temperature = 0;
    Humidity = 0;
  }

  if(isnan(outsideTemp) || isnan(outsideHum)){
    outsideTemp = 0;
    outsideHum = 0;
    outsideState = 2;
  }
  if(isnan(upstairsTemp) || isnan(upstairsHum)){
    upstairsTemp = 0;
    upstairsHum = 0;
    upstairsState = 2;
  }

  //Make json object with data
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& room1 = root.createNestedArray("room1");
  JsonArray& room0 = root.createNestedArray("room0");
  JsonArray& room2 = root.createNestedArray("room2");
  room1.add(Temperature);
  room1.add(Humidity);
  
  room0.add(outsideTemp);
  room0.add(outsideHum);
  room0.add(outsideState);
  room0.add(outsideUpdate);
  
  room2.add(upstairsTemp);
  room2.add(upstairsHum);
  room2.add(upstairsState);
  room2.add(upstairsUpdate);
  String json;
  root.prettyPrintTo(json);
  server.send(200, "application/json", json);
}

//Relay switch function
void handle_lightsSwitch(){
  
  lightsStatus = !lightsStatus;
 
  WiFiClient client;
  const char * host = "192.168.0.101"; //Relay controlling esp ip adress
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection to host 192.168.0.101 failed");
    return;
  }

  // We now create a URI for the request. Something like /data/?sensor_reading=123
  String url = "/lightsChange/";
  url += "?button_reading=";
  url += lightsStatus;

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "Connection: close\r\n\r\n");
  client.stop();
  Serial.println(lightsStatus);
  String lightsStat = static_cast<String>(lightsStatus);
  server.send(200, "text/plain", lightsStat);
}

//Function to handle sended data
void handleSentVar() {
  
  //Receive variables from other esp
  if (server.hasArg("sensor_reading"))
  {
    sensor_values = server.arg("sensor_reading");
  }
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(sensor_values);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  if (root.success()) {
    int roomNum = root["esp_num"];
    if(roomNum == 0){
      outsideTemp = root["dhtTemp_reading"].as<float>();
      outsideHum = root["dhtHum_reading"].as<float>();
      outsideUpdate = 0;
      outsideState = 1;
    }if(roomNum == 2){
      upstairsTemp = root["dhtTemp_reading"].as<float>();
      upstairsHum = root["dhtHum_reading"].as<float>();
      upstairsUpdate = 0;
      upstairsState = 1;
    }
  }
}

//Html, css, js code
String SendHTML(){
  String html ="<!DOCTYPE html> <html id=\"webpage\"> <head> <meta charset=\"utf-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\"> <title>ESP8266 Main-hub</title> <style>@import url('https://fonts.googleapis.com/css2?family=Gemunu+Libre:wght@300;500;600&display=swap'); html { font-size: 4vw; font-family: 'Gemunu Libre', sans-serif; } body { color: #fff; background-color: #222; } /* Main body */ body main { display: flex; flex-wrap: wrap; justify-content: space-around; } body main h1 { font-weight: 600; font-size: 2rem; width: 80%; text-align: center; cursor: default; } body main h1::before { content: \"\"; display: block; margin: 0 0 2% 10%; width: 30%; height: 4px; background-color: #fdd870; border-radius: 5px; transition: 1s; } /* Rooms styles */ body main .room { background-color: #303030; display: flex; flex-flow: column; width: 90%; height: 70vw; box-shadow: 0 0 10px black; border-radius: 2px; margin: 3% 0 3% 0; font-size: 1.2rem; } body main .room h2 { display: flex; align-items: center; background-color: #fdd870; border-radius: 2px; margin: 0; padding-left: 10%; font-size: 1.5rem; font-weight: 500; height: 15%; } /* Sensors section */ body main .room .sensors { display: flex; flex-flow: row; justify-content: center; padding: 2% 2% 0 2%; height: 40%; } body main .room .sensors .temperature, .humidity { display: flex; flex-wrap: wrap; justify-content: center; align-content: space-around; width: 50%; } body main .room .sensors .temperature h3, .humidity h3 { width: 100%; text-align: center; font-weight: 300; margin: 0; } body main .room .sensors .temperature p, .humidity p { width: 100%; text-align: center; font-size: 2rem; font-weight: 300; margin: 0; } body main .room .sensors div .progress-bar { width: 80%; display: flex; align-items: center; height: 10%; background-color: #222; border-radius: 30px; border: 1vw solid #222; box-shadow: inset 0 1px 3px rgba(0, 0, 0, .2); } body main .room .sensors div .progress-bar-fill { display: block; height: 100%; background-color: #f9a84c; border-radius: 20px; transition: width 500ms ease-in-out; } body main .room .sensors .humidity .progress-bar-fill { background-color: #009dff; } /* Relays section */ body main .room .relays { display: flex; flex-flow: column; flex-wrap: wrap; justify-content: center; align-items: center; padding: 2% 2% 0 2%; height: 40%; } body main .room .relays h3 { width: 50%; text-align: center; margin: 0% 0 5% 0; font-weight: 300; } /* Room keria132 */ body main .keria132 .relays .lights-button { width: 20%; height: 30%; margin: 0% 0% 5% 0%; background: #222; border: 2px solid black; border-radius: 40px; outline: none; cursor: pointer; -webkit-appearance: none; -moz-appearance: none; appearance: none; transition: all 0.3s cubic-bezier(0.2, 0.85, 0.32, 1.2); } body main .keria132 .relays .lights-button::after { content: \"\"; display: inline-block; left: 3px; width: 55%; height: 100%; background-color: #ff4d00; border-radius: 50%; transition: all 0.1s cubic-bezier(0.2, 0.85, 0.32, 1.2); transform: translateX(0); } body main .keria132 .relays .lights-button:checked::after { transform: translateX(calc(80%)); background-color: #ffcf66; } body main .keria132 .relays .power-button { width: 20%; height: 30%; margin: 0% 0 5% 0; } /* Outside section style */ body main .outside h2 { background-color: #8cd3ff; height: 20%; } body main .outside p.outsideStatus { align-self: flex-end; margin-right: 5%; } /*Upstairs style*/ body main .upstairs h2 { background-color: #ff4d00; height: 20%; } body main .upstairs p.upstairsStatus { align-self: flex-end; margin-right: 5%; } /* Media queries */@media screen and (min-width: 1000px) { html { font-size: 2vw; } body main .room { width: 45%; height: 40vw; } } </style> </head> <body> <main> <h1> ESP8266<br>Main Hub </h1> <section class=\"room keria132\"> <h2>Room №1</h2> <div class=\"sensors\"> <div class=\"temperature\"> <h3>Temperature:</h3> <p class=\"temperature_text\" id=\"temperature_room1\"> 0°C </p> <div class=\"progress-bar\"> <span class=\"progress-bar-fill\"></span> </div> </div> <div class=\"humidity\"> <h3>Humidity:</h3> <p class=\"humidity_text\" id=\"humidity_room1\"> 0% </p> <div class=\"progress-bar\"> <span class=\"progress-bar-fill\"></span> </div> </div> </div> <div class=\"relays\"> <h3>Lights</h3> <input type=\"checkbox\" class=\"lights-button\" onClick=\"toggleCheckbox()\"";
  if(lightsStatus)html += "checked";
  html += "> <h3>Power</h3> <input type=\"checkbox\" class=\"power-button\"> </div> </section> <section class=\"room outside\"> <h2>Outside</h2> <div class=\"sensors\"> <div class=\"temperature\"> <h3>Temperature:</h3> <p class=\"temperature_text\" id=\"temperature_room0\"> 0°C </p> <div class=\"progress-bar\"> <span class=\"progress-bar-fill\"></span> </div> </div> <div class=\"humidity\"> <h3>Humidity:</h3> <p class=\"humidity_text\" id=\"humidity_room0\"> 0% </p> <div class=\"progress-bar\"> <span class=\"progress-bar-fill\"></span> </div> </div> </div> <div class=\"relays\"></div> <p class=\"outsideStatus\"></p> </section> <section class=\"room upstairs\"> <h2>Upstairs</h2> <div class=\"sensors\"> <div class=\"temperature\"> <h3>Temperature:</h3> <p class=\"temperature_text\" id=\"temperature_room0\"> 0°C </p> <div class=\"progress-bar\"> <span class=\"progress-bar-fill\"></span> </div> </div> <div class=\"humidity\"> <h3>Humidity:</h3> <p class=\"humidity_text\" id=\"humidity_room0\"> 0% </p> <div class=\"progress-bar\"> <span class=\"progress-bar-fill\"></span> </div> </div> </div> <div class=\"relays\"></div> <p class=\"upstairsStatus\"></p> </section> </main> </body> <script> setInterval(loadDoc, 1000); let progressBars = document.querySelectorAll('.progress-bar-fill') , roomsTemp = document.querySelectorAll(\".temperature_text\") , roomsHum = document.querySelectorAll(\".humidity_text\") , room0 = document.querySelector(\".outside\") , room0Status = document.querySelector(\".outsideStatus\") , room2 = document.querySelector(\".upstairs\") , room2Status = document.querySelector(\".upstairsStatus\"); function loadDoc() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { let data = JSON.parse(this.responseText); roomsTemp[0].textContent = data.room1[0] + \"°C\"; roomsHum[0].textContent = data.room1[1] + \"%\"; progressBars[0].style.width = data.room1[0] * 2 + \"%\"; progressBars[1].style.width = data.room1[1] + \"%\"; roomsTemp[1].textContent = data.room0[0] + \"°C\"; roomsHum[1].textContent = data.room0[1] + \"%\"; progressBars[2].style.width = data.room0[0] * 2 + \"%\"; progressBars[3].style.width = data.room0[1] + \"%\"; roomsTemp[2].textContent = data.room2[0] + \"°C\"; roomsHum[2].textContent = data.room2[1] + \"%\"; progressBars[4].style.width = data.room2[0] * 2 + \"%\"; progressBars[5].style.width = data.room2[1] + \"%\"; if (data.room0[2] == 0) { room0.style.opacity = \"0.4\"; room0Status.innerHTML = \"ESP is offline\"; } else if (data.room0[2] == 2) { room0.style.opacity = \"0.4\"; room0Status.innerHTML = \"Data is corrupted!\"; } else { room0.style.opacity = \"1\"; room0Status.innerHTML = \"Last update: \" + data.room0[3] + \"s\"; } if (data.room2[2] == 0) { room2.style.opacity = \"0.4\"; room2Status.innerHTML = \"ESP is offline\"; } else if (data.room2[2] == 2) { room2.style.opacity = \"0.4\"; room2Status.innerHTML = \"Data is corrupted!\"; } else { room2.style.opacity = \"1\"; room2Status.innerHTML = \"Last update: \" + data.room2[3] + \"s\"; } console.log(data); } } ; xhttp.open(\"GET\", \"/update_data\", true); xhttp.send(); } function toggleCheckbox(x) { var xhr = new XMLHttpRequest(); xhr.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { console.log(this.responseText, document.querySelector(\".lights-button\")); let lightsStatus = this.responseText; if (this.responseText == 0) { document.querySelector(\".lights-button\").checked = false; } else { document.querySelector(\".lights-button\").checked = true; } } } ; xhr.open(\"GET\", \"/lightsSwitch\", true); xhr.send(); } </script> </html>";
  return html;
}
