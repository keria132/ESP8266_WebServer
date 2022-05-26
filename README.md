# ESP8266_WebServer
ESP8266 server with simple communication between other esp8266 devices.

//How it works
1. Main server receives DHT sensors data from esp0 and esp1 and displays it.
2. Also sends requests to esp_relay to switch it when you press switch button on the website.
//
To run this you need to upload code in 4 different esp8166 devices.
In my case 3 of them (including main server) colect data from dht sensors and one controls relay
