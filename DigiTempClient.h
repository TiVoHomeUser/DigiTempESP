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
	#if AUTOCONNECT  // AutoConnect obtain credentials from web page
	  WiFiManager wm;
	  wm.autoConnect(MYHOSTNAME"_AP"); 								// auto generated AP name from chipid
	#else  			// Hard-coded connection
	  WiFi.begin(ssid, password);
	#endif

	  Serial.println("");


	  // Wait for connection
	  while (WiFi.status() != WL_CONNECTED) {
	    delay(500);
	    Serial.print(".");
	  }
	  Serial.println("");
	  Serial.print(F("Connected to "));
	  Serial.println(ssid);
	  Serial.print(F("IP address: "));
	  Serial.println(WiFi.localIP());
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
    sprintf(highlow, "High: %02.2fF\t\tLow: %02.2fF",Th_t.tmax,Th_t.tmin);
    content.replace("{{HighLow}}", String(highlow));
   }
  content.replace("{{MyIP}}", String(WiFi.localIP().toString()));
  content.replace("{{UID}}", SUID);
  content.replace("{{myHostName}}",hostName);

  Server.send(200, "text/html", content);
}


void my_setup() {
	hostName = MY_HOSTNAME;
	setupClient();
	Serial.print("Hello from DigiTemp Client ");
	Serial.println(hostName);
	Serial.println(client_copyrite);
	Serial.println(client_compiledate);
	// Behavior a root path of ESP8266WebServer.
	Server.on("/", rootPage);
	Server.on("/start", startPage);   // Set NTP server trigger handler
	Server.on("/getData", send_getData);  // Server read data and Keep Alive
	Serial.println(Server.client().localIP().toString());
}


void my_loop() {
//  if(timeElapsed()){
//    //Serial.print('.');
//    loopDHT();
//  }
//	Server.handleClient();
}


#endif
