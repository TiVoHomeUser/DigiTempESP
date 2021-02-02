#ifndef DIGITEMP_H
#define DIGITEMP_H

#define copyrite " &#169; Jan 2021 VictorWheeler myapps@vicw.net use, modify and distribute without restrictions"
#define compiledate __DATE__

#ifndef APSSID
#define APSSID  "DigiTempESP"
#define APPSK   "DigiTempPSK"
#endif

#if SERVER
#define MY_HOSTNAME "DigiTemp-AP"          // Name for this client#define MY_HOSTNAME "DigiTemp-02"          // Name for this client
#else
#define MY_HOSTNAME "DigiTemp-02"          // Name for this client
#endif

static const String SUID = String(ESP.getChipId());
unsigned int suid = ESP.getChipId();

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#endif

// Server is used for both Server and Client
#if defined(ARDUINO_ARCH_ESP8266)
ESP8266WebServer Server;
#elif defined(ARDUINO_ARCH_ESP32)
WebServer Server;
#endif

boolean DEBUG = true;	// Extra output to serial monitor
boolean BiLED_DEBUG = false;

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

void setupSerial(){
	delay(1000);
	Serial.begin(74880); //115200);
	Serial.println("");
	Serial.println("Copyright "); Serial.print( copyrite );
	Serial.print(" "); Serial.println(compiledate);
	Serial.print(String(Server.uri()));
	Serial.print("\n\rDigiTempESP " + SUID);
#if SERVER
	Serial.print(" Host");
#else
	Serial.print(" Client");
#endif
	Serial.println(" Mode");
}

#include <DHT.h>
#define DHTPIN 4     	// what digital pin the DHT22 is conected to
                     	// D4 = D2 om nodemcu D2 = D2 on D2 mini lite
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors
//#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

String hostName;		// Name for Server or Client

// storage for temperatue and Humidity
typedef  struct Th_temp{
  float h = 0.0;
  float c = 0.0;
  float f = 0.0;
  float hif = 0.0;
  float hic = 0.0;
  float tmax = -999.99;
  float tmin = 999.99;
} Th_temp;
Th_temp Th_t;

const int led = 13;   	// LED digital output
static int ledOn = 0;       // Led toggles each time temperature is read
void setupBiLED(){
	  Serial.print("LED_BUILTIN "); Serial.print(LED_BUILTIN);
	  Serial.print("\tOUTPUT "); Serial.println(OUTPUT);
	  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
	  pinMode(led, OUTPUT);
	  digitalWrite(led, 0);
}

void setupDHT() {
  dht.begin(55);	// default 55usec
}

/*
 *
 * 			Change Built In LED state (Show we are still alive)
 *
 */
void toggleBiLED(){
if (ledOn > 0) {
	digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
	if(DEBUG && BiLED_DEBUG) Serial.print(F("ON "));
	ledOn = -1;
} else {
	digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
	if(DEBUG && BiLED_DEBUG) Serial.print(F("OFF "));
	ledOn = 1;
}
if(DEBUG && BiLED_DEBUG) Serial.flush();

}
/*
 *
 *                        loopDHT()
 *
 *
 */
static int loopCountDHT = 0;
void loopDHT() {
	loopCountDHT++;   // Assuming 1 second loops

	// Report every 20 seconds.
	if (loopCountDHT > 20) { //15000) { loop cycle = 1 second
		toggleBiLED();

		// Reading temperature or humidity takes about 250 milliseconds!
		// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
		Th_t.h = dht.readHumidity();
		if(DEBUG) { Serial.print(F("\ndht.readHumidity: ")); Serial.print(Th_t.h); }

		// Read temperature as Celsius (the default)
		Th_t.c = dht.readTemperature();
		if(DEBUG) { Serial.print("\tdht.readTemperature(false): "); Serial.print(Th_t.c); }

		// Read temperature as Fahrenheit (isFahrenheit = true)
		Th_t.f = dht.readTemperature(true);
		if(DEBUG) { Serial.print(F("\tdht.readTemperature(true): ")); Serial.println(Th_t.f); }

    	// Check if any reads failed and exit early (to try again).
    	if (isnan(Th_t.h) || isnan(Th_t.c) || isnan(Th_t.f)) {
    		Serial.println(F("Failed to read from DHT sensor!"));
    		// Server.stop();
    		loopCountDHT = 0;
    		return;
    	}

    	// Compute heat index in Fahrenheit (the default)
    	Th_t.hif = dht.computeHeatIndex(Th_t.f, Th_t.h);

    	// Compute heat index in Celsius (isFahreheit = false)
    	Th_t.hic = dht.computeHeatIndex(Th_t.c, Th_t.h, false);

    	// Track High / Low
    	if (Th_t.f > Th_t.tmax) {
    		Th_t.tmax = Th_t.f;
    	}
    	if (Th_t.f < Th_t.tmin) {
    		Th_t.tmin = Th_t.f;
    	}
    	loopCountDHT = 0;
	} // loopCount = true
}

/*
 *
 *  instead of using delay() use the timmer for loop at 1 second stuff
 *
 */
 unsigned long lastmills = 0;
 const unsigned long secondIntervill = 1000;
 // const unsigned long minuteIntervill = secondIntervill * 60;
 // const unsigned long hourIntervill = minuteIntervill * 60;
 // const unsigned long dayIntervill = hourIntervill * 24;

unsigned long currentmills = secondIntervill;  // do it on first loop
boolean timeElapsed(){
  currentmills = millis();
  if(currentmills - lastmills >= secondIntervill){
    lastmills = currentmills;
    return true;
  }
  return false;
}

/*

   Basic web page to display temperature and humidity
   Accessed by DigiTemp server for both data and keep alive

*/
String getData() {
	if(DEBUG) Serial.println("Hello from getData()");
  String  content;
  content  = "<!DOCTYPE html>\n";
  content += "<html>\n";
  content += "<head>\n";
  content += "<meta charset=\"UTF 8\">\n";
  content += "<title>DigiTemp data cookie {{myHostName}} </title>\n";
  content += "</head>\n";
  content += "<body>\n";
  //  content += "<p>";
  content += " Sensor: {{UID}} <br>\n";
  content += " {{MyIP}}  <br>\n";
  content += "Humidity: {{Humid}} <br>\n";
  content += "Temperature: {{TempF}} <br>\n";
  content += "Temperature: {{TempC}} <br>\n";
  content += "Temperature_Max: {{TempMax}} <br>\n";
  content += "Temperature_Min: {{TempMin}} <br>\n";

  //  content += "</p>\n";
  content += "Hostname: {{myHostName}}<br>\n";
  content += "<a href=\"http://{{DiGiTempServerIP}}/\">DigiTemp Server</a>\n";
  content += "</body>";
  content += "</html>";

  char humid[20];
  char tempC[20];
  char tempF[20];
  char tempMax[20];
  char tempMin[20];
  sprintf(tempC, "%02.2fC", Th_t.c);
  sprintf(tempF, "%04.2fF", Th_t.f);
  sprintf(humid, "%02.1f%%", Th_t.h);
  sprintf(tempMax, "%04.2fF", Th_t.tmax);
  sprintf(tempMin, "%04.2fF", Th_t.tmin);
  content.replace("{{TempC}}", String(tempC));
  content.replace("{{TempF}}", String(tempF));
  content.replace("{{TempMax}}", String(tempMax));
  content.replace("{{TempMin}}", String(tempMin));
  content.replace("{{Humid}}", String(humid));
  content.replace("{{UID}}", SUID);
  content.replace("{{MyIP}}", Server.client().localIP().toString()); //String(WiFi.localIP().toString()));
  content.replace("{{myHostName}}", hostName);
  
  // Must be a better way to find the IP of the access point WiFi softAPSSID is good except Microsoft does not always support mDNS
  // Might try go-back except might of entered directly need to find host if !using AP 
  #if SERVER
  content.replace("{{DiGiTempServerIP}}", WiFi.softAPIP().toString());  
  #else
  content.replace("{{DiGiTempServerIP}}", WiFi.gatewayIP().toString());
  #endif
  //WiFi.SSID()); //Server.client().localIP().toString()); //String(WiFi.gatewayIP().toString())); // DigiTemp Server
  //content.replace("{{DiGiTempServerIP}}", WiFi.softAPSSID()); // DigiTemp Server
  Server.send(200, "text/html", content);
  Server.client().flush();
  if(DEBUG)
  Serial.println("getData() Called");
  Serial.println(WiFi.BSSIDstr());
  Serial.println(WiFi.SSID());	//*
  Serial.println(WiFi.hostname());
  Serial.println(WiFi.psk());
  Serial.println(WiFi.softAPSSID()); //**
  Serial.println(WiFi.softAPmacAddress());
  Serial.println(WiFi.softAPIP().toString());
  Serial.print(hostName); Serial.println(".local");  
  Serial.println( Server.uri());
  Serial.println(Server.client().localIP().toString());
  Serial.println(Server.client().remoteIP().toString());
  Serial.println(Server.urlDecode(WiFi.SSID()));  //*
  Serial.println(WiFi.gatewayIP().toString());

  return content;
}

void send_getData(){
	Server.send(200, "text/html", getData());
	//Server.client().stop();
	Server.client().flush();

}

/*
 *                          StartPage 
 * 
 */
void startPage() {
  if(DEBUG) Serial.println("Hello from startPage()");
  // Retrieve the value of AutoConnectElement with arg function of WebServer class.
  // Values are accessible with the element name.
  // it redirects to the root page without the content response.
  Server.sendHeader("Location", String("http://") + Server.client().localIP().toString() + String("/"));
  Server.send(302, "text/plain", "");
  Server.client().flush();
  Server.client().stop();
}

/*
 *                    Page Not Found
 * 
 */
void notFoundPage() {
	if(DEBUG){
		Serial.print(Server.uri());
		Serial.println(F(" not found"));
	}
    Server.send(404, F("text/plain"), F("404: Not found"));
	Server.client().stop();
}


#if SERVER
#include "digiTempServer.h"
#else
#include "digiTempClient.h"
#endif


#endif		// DIGITEMP_H
