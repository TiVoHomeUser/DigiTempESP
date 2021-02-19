
/*
 *
 * Client for ESP8266 with DHT sensor
 * Auto connect with last used network able to enter new SSID and Passphrase
 *
 * Victor Wheeler myapps@vicw.net
 *
 */
#define copyrite " &#169; Jan 2021 VictorWheeler myapps@vicw.net use, modify and distribute without restrictions"
#define compiledate __DATE__
#include "Arduino.h"

// To build for Host set to true set to false for Client
//#define SERVER true		// server or client


#include "DigiTempESP.h"



void setup(){
	setupSerial();
	setupBiLED();		// Built-in LED
	setupDHT();     	// DigiTemp setup for the hardware sensor device
	my_setup();			// Setup for either server or client
	Server.on("/", rootPage);
	Server.on("/getData", send_getData);  // Server read data and Keep Alive
	Server.on("/reboot", reboot);
	Server.onNotFound(notFoundPage);
	// TODO /favicon.ico not found
	Server.begin();
}


void loop() {
	if (timeElapsed()) {	// 1 Second
		toggleBiLED();
		loopDHT();
		still_here++;
	}

	if (Serial.available() > 0) {
		Serial.println();
		Serial.print(ESP.getChipId());
		Serial.print(F(" Runnig as "));
    	#if SERVER
			Serial.println(F("Host "));
    	#else
			Serial.println(F("Client "));
    	#endif
      char r = Serial.read();
      do_serial(r);
	} // Serial.avaiable()
	my_loop();	// either Server or Client loop()
	Server.handleClient(); // Handle network requests
}
