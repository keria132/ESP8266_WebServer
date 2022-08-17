# ESP8266_AsyncWebServer
//Just my personal project i did for myself out of curiosity;
//Coded in Arduino IDE;

It's ESP8266 server with simple communication between other esp8266 devices.
Shows DHT data from all the devices and controls relays on them;
!NOTE: Relay switches function doesn't exist for now

## How it works:
* Main server receives DHT sensors data from ESP devices(rooms) and displays it.
And also acts as the main room with DHT sensor and relay switches ()
* Sends requests to esp-device to switch relay on it when you press switch button on the website.
* To run this you need to simply upload main server to your esp8266 with DHT sensor and then upload
html and css files to SPIFFS (data folder). And optionaly you can upload esp_device sketch to
another esp with DHT. Don't forget to set up pins and wifi ssid/password;
