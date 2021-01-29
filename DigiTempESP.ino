
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

#define SERVER false		// server or client


#include "DigiTempESP.h"



void setup(){
	setupSerial();
	setupBiLED();		// Built-in LED
	setupDHT();     	//  DigiTemp setup sensor
  Server.onNotFound(handleNotFound);
	my_setup();			// Either server or client setup()
	Server.begin();
}


void loop() {
	if (timeElapsed()) {
		toggleBiLED();
		loopDHT();
	}
	my_loop();	// either Server or Client loop()
	Server.handleClient();
}
