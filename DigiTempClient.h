#ifndef DIGITEMP_CLIENT
#define DIGITEMP_CLIENT

/*
 *
 * Client for ESP8266 with DHT sensor
 * Auto connect with last used network able to enter new SSID and Passphrase
 *
 * Victor Wheeler myapps@vicw.net
 *
 */
#define client_copyrite " &#169; Jan 2021 VictorWheeler myapps@vicw.net use, modify and distribute without restrictions"
#define client_compiledate __DATE__
#include "DigiTempESP.h"

void setupClient(){
	 WiFi.mode(WIFI_STA);
	#if AUTOCONNECT                   // AutoConnect obtain credentials from web page
	  WiFiManager wm;
	  wm.autoConnect(MYHOSTNAME"_AP");
	#else  			                      // Hard-coded connection
	  WiFi.begin(ssid, password);     // auto generated AP name from chipid
Serial.println(ssid);
Serial.println(password);
	#endif

	  Serial.println("");

	  // Wait for connection
	  while (WiFi.status() != WL_CONNECTED) {
	    delay(250); yield();
	    Serial.print(".");
      delay(250); yield();
	  }
	  Serial.println("");
	  Serial.print(F("Connected to "));
	  Serial.println(ssid);
	  Serial.print(F("IP address: "));
	  Serial.println(WiFi.localIP());
    Serial.print(F("Soft IP address: "));
    Serial.println(WiFi.softAPIP().toString());
    Serial.print(F("GateWay IP address: "));
    Serial.println(WiFi.gatewayIP().toString());
}

void rootPage() {
	if(DEBUG) Serial.println("Hello from rootPage()");
  String  content;
  if(isnan(Th_t.c)){
      content =
      "<html>"
      "<head>"
      "<meta name=\"R_LabDigiTmp\" content=\"width=device-width, initial-scale=1\">"
      "<script type=\"text/javascript\">"
      "setTimeout(\"location.reload()\", 30000);"
      "</script>"
      "</head>"
      "<body>"
      "<h2 align=\"center\" style=\"color:blue;margin:20px;\">You are connected</h2>"
      "<h2 align=\"center\" style=\"color:lightgray;margin:20px;\">{{MyIP}} {{myHostName}}</h2>"
      "</body>"
      "</html>";
  } else {
    content=
    "<html>"
    "<head>"
    "<meta name=\"R_LabDigiTmp\" content=\"width=device-width, initial-scale=1\">"
    "<script type=\"text/javascript\">"
    "setTimeout(\"location.reload()\", 15000);"
    "</script>"
    "</head>"
    "<body>"
    "<h2 align=\"center\" style=\"color:lighttgray;margin:20px;\">{{MyIP}} {{myHostName}}</h2>"
    "<h3 align=\"center\" style=\"color:blue;margin:15px;\">{{TempHumi}}</h3>"
    "<h3 align=\"center\" style=\"color:gray;margin:15px;\">{{HighLow}}</h3>"
    "</body>"
    "</html>";

     char tempHumi[80];
     char highlow[40];
     // "Humidity: 44.60% Temperature: 22.40 *C 72.32 *F  Heat index: 21.86 *C 71.35 *F
    sprintf(tempHumi,"Humidity: %02.1f%%\t\tTemperature: %02.2fC / %02.2fF\t\tHeat index: %04.2fC / %04.2fF",
    Th_t.h, Th_t.c, Th_t.f, Th_t.hic, Th_t.hif);
    content.replace("{{TempHumi}}", String(tempHumi));
    if(scale){
        sprintf(highlow, "High: %02.2fC\t\tLow: %02.2fC",dht.convertFtoC(Th_t.tmaxf),dht.convertFtoC(Th_t.tminf));
    } else {
    	sprintf(highlow, "High: %02.2fF\t\tLow: %02.2fF",Th_t.tmaxf,Th_t.tminf);
    }
    content.replace("{{HighLow}}", String(highlow));
   }
  content.replace("{{MyIP}}", String(WiFi.localIP().toString()));
  content.replace("{{UID}}", SUID);
  content.replace("{{myHostName}}",hostName);

  Server.send(200, "text/html", content);
  Server.client().flush();
}


void my_setup() {
	hostName = MY_HOSTNAME;
	setupClient();
	Serial.print("Hello from DigiTemp Client ");
	Serial.println(hostName);
	Serial.println(client_copyrite);
	Serial.println(client_compiledate);
	Serial.println(Server.client().localIP().toString());
}

/*
 *
 * 				Kind of a watch-dog if this client has not been accessed in 300 seconds
 * 				Re establish the connection.
 *
 *
 */
// int still_here defined and reset in getData()
boolean still_alive(){
	boolean retval = true;
	if(still_here > 300){	// Number of seconds to try reconnect
		if(DEBUG){Serial.print(F("Reseting connection to host still_here = ")); Serial.println(still_here);}
		if(WiFi.reconnect()) {
			still_here = 0;
		}  else {
			retval = false;
		}
	}
	return retval;
}

void my_loop() {
	if(!still_alive()){
		Serial.println(F("*** Connection lost ***"));
	}
}

/*
 * 		Process any serial monitor input for this client
 */
void do_serial(char r){
  if(r == '?') loopDHT();		// Need to do something with r may as well force update data
  Serial.print(hostName);
  Serial.print("\t");
  Serial.print(WiFi.localIP().toString());
  Serial.print("\t");
  Serial.println(client_compiledate);
}

#endif
