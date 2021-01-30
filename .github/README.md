# DigiTempESP
Monitor several ESP8266+DHT22 sensors from a single web page.
This project is 2 parts a Host and Client the Host creates an access point and collects Temperature And Humidity from up to 7 clients displays the information in a single web page. The clients have a DHT22 or similar sensor attached is able to send the data to the host the host can have a sensor attached also.

 Root Page 
![Root Page](./DiGiTempESP.png)

After setting up the first ESP8266 + DHT22 temperature sensor I built a second for the other side of the room I wanted to monitor both from a single device like a smart phone so I setup one of the devices as an access point capable of serving an html page on the local network that monitors one or more of the temperature sensors. This AP registers a hostname using mDNS I used DigiTemp for the host name allowing me to access the monitoring device using the URL http://DigiTemp-ap.local .  I have tested with 4 sensors the maximum number of concurrent connections stated for ESP8266WiFi is 8 therefore it should work with up to 7 sensors leaving 1 connection for the smart phone, tablet or PC.    

I broke the digitemp.ino sketch into several smaller and easier to manage files I found using the .h extension allows editing using the Arduino IDE with the syntax checking and highlighting they are not actual ’C’ header files.
 
 - DigiTempESP.ino	Arduino sketch launching point
 - DigiTempESP.h	Common related to both Host and Client
 - DitiTempServer.h	Functions for the Host 
 - DigiTempClien.h	Functions for the Client
