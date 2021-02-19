#ifndef User_Settings_ino
#define User_Settings_ino "Feb 14, 2021"

#ifndef APSSID
#define APSSID	"DigiTempESP"	// This WiFi Access Point name
#define APPSK   "DigiTempPSK"	// This WiFi AP password
#endif

// To build for Host set to true set to false for Client
#define SERVER false		// server or client
#if SERVER
#define MY_HOSTNAME "DigiTemp-AP"          // Name for this client#define MY_HOSTNAME "DigiTemp-02"          // Name for this client
#else
#define MY_HOSTNAME	"DigiTemp-07"          // Name for this client
#endif


#include <DHT.h>
#define DHTPIN 4     	// what digital pin the DHT22 is conected to
                     	// D4 = D2 om nodemcu D2 = D2 on D2 mini lite
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors
//#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#endif
