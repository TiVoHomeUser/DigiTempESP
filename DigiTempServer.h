#ifndef DIGITEMPSERVER_H
#define DIGITEMPSERVER_H

#define server_copyrite " &#169; Jan 2021 VictorWheeler myapps@vicw.net use, modify and distribute without restrictions"
#define server_compiledate __DATE__

#include "DigiTempESP.h"
#include <ESP8266mDNS.h>        // mDNS Client or Bonjour Client library for the esp8266.
// For lookup using hostname.local instead of IP address



const int max_connection = 8;
const int channel = 4;

// OhOh can't delay or print in a event get out of event using the newClient flag.
boolean newClient = true; // First run We are new!



typedef struct {
  IPAddress ipAddress;
  boolean   isAlive; //validIP;
  String    myHostName;
  Th_temp   Th_t;
  unsigned long last_check;	// Last time sensor was accessed
} clist;

clist clients[max_connection + 2];    // include space for AP if DHT is enabled

// WiFiEventSoftAPModeStationConnected  Manage incoming device connection on ESP access point
void onNewStation(WiFiEventSoftAPModeStationConnected sta_info) {
  if(DEBUG){
	  Serial.print("New Station : ");
	  Serial.print(sta_info.aid);
	  Serial.print(" ");
  }

  newClient = true;
}

//WiFiEventStationModeDisconnected
void onRemoveStation(WiFiEventSoftAPModeStationDisconnected sta_info) {
  if(DEBUG){
	  Serial.println("Station Removed: ");
	  Serial.print(sta_info.aid);
	  Serial.print(" ");
  }
  newClient = true;
}


/*
 *
 *  Setup wifi as a soft Access Point
 *
 */
void setupAP() {
  Serial.println("setupAP");
  Serial.print("\n\rDigiTemp AP max "); Serial.print(max_connection); Serial.println(" WiFi connections");
  WiFi.softAP(ssid, password, channel, false, max_connection);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: "); Serial.println(myIP);

  // Event subscription
  static WiFiEventHandler e1 = WiFi.onSoftAPModeStationConnected(onNewStation);
  static WiFiEventHandler e2 = WiFi.onSoftAPModeStationDisconnected(onRemoveStation);
}

/*
 *
 *   Find the substring "key" in the string s and return the value associated with it.
 *   value is the chars following the white space after the key
 *
 */
float  parser(const String* s, const char* key) {
  float retVal = NAN;

  // Search for key
  if (s->indexOf(key) >= 0) {   // Found key
	unsigned int ptr = s->indexOf(key);
    //compute end of key
    ptr += strlen(key);

    // remove white space
    while (isspace(s->charAt(ptr)) && ptr < s->length()) {
      ptr++;
    } // ptr now at start of value or EOLN
    unsigned int stptr = ptr;    // start of the value

    // find end of value
    while ((isdigit(s->charAt(ptr)) || s->charAt(ptr) == '.') && ptr < s->length()) {
      ptr++;
    }
    if(ptr > stptr){ 	// Not a number
    	retVal = s->substring(stptr, ptr).toFloat();
    }
  }
  return retVal;
}

boolean checkLocalStation(clist* addss) {
Serial.print(F(" checkLocalStation() "));
 // addss->last_check = millis();
  addss->Th_t.h = Th_t.h;
  addss->Th_t.f = Th_t.f;
  addss->Th_t.c = Th_t.c;
  addss->Th_t.tmax = Th_t.tmax;
  addss->Th_t.tmin = Th_t.tmin;
  addss->myHostName = hostName;
  return true;
}


/**
 *                                                 checkStation()
 *                  verify active stations client array still exists and fill structure clist with data
 *
 *
 */
//boolean checkStation(IPAddress stationIP){
boolean checkStation(clist* addss) {
  //boolean checkStation(int index) {
  //  clist* addss = &clients[index];
  //addss->isAlive = true;

  addss->last_check = millis();

Serial.print(" Hello from checkStation ");
  if (addss->ipAddress == WiFi.softAPIP()) {
    return checkLocalStation(addss);    // local host url not working
  }

  String url = "/getData";
  String host = addss->ipAddress.toString();

  Serial.print("Connecting to ");
  Serial.print(host + url);

  WiFiClient client;
  //Serial.print("looking for 404 ");  //Serial.println(client.connect(host, 80));
  if (!client.connect(host, 80)) {
    Serial.print(" Connection failed "); Serial.println(client.status());
    addss->isAlive = false;
    return false;
  } else {
    Serial.println();
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n" +
               "\r\n"
              );
  delay(10);
  int ptr;
  int eoln;
  float temp;
  String temps;

  Serial.println("Trying to read " + host + url);
  while (client.connected() || client.available()) {
    if (client.available()) {

  	  String line = client.readStringUntil('\n');

      if(line.indexOf("404") > 0){
        Serial.println(F(" ************************** 4 0 4 ********************************* "));
        addss->isAlive = false;
        client.stop();
        return false;
      } else {
//      if (line.indexOf("404 Not found") == -1) {

        // Humidity
        ptr = line.indexOf("Humidity:");    // Quick check for now need to modify parser to return boolean and something else for the retVal
        if (ptr >= 0) {
          temp = parser(&line, "Humidity:");
          addss->Th_t.h = temp;
        }

        // Temperature (both)
        ptr = line.indexOf("Temperature:");
        if (ptr >= 0) {
          temp = parser(&line, "Temperature:");
          ptr = line.indexOf("F");
          if (ptr >= 0)
            addss->Th_t.f = temp;
          else
            addss->Th_t.c = temp;
        }

        ptr = line.indexOf("Temperature_Max:");
        if (ptr >= 0) {
          temp = parser(&line, "Temperature_Max:");
          addss->Th_t.tmax = temp;
        }

        ptr = line.indexOf("Temperature_Min:");
        if (ptr >= 0) {
          temp = parser(&line, "Temperature_Min:");
          addss->Th_t.tmin = temp;
        }

        // Hostname
        ptr = line.indexOf("Hostname:");
        if (ptr >= 0) {
          eoln = line.length();
          if (eoln > ptr) { // JIC WTGBIN
            temps = line.substring((ptr + 10), eoln);
            // Serial.println("Found for Hostname: " + temps);
            addss->myHostName = temps;
          }
        }

      }
//      else {
//        Serial.println( " 404 found bypass");
//        addss->isAlive = false;
//        client.stop();
//        return false;
//      }
    } // else { Serial.print('.\r');} //"client.available = false");}
    yield();
  } // while connected
  client.stop();
  return true;
}

/*
 *
 *         load client array with IP and isAlive boolean.
 *         reset = false do not change client's isAlive status
 *
 */
void Load_Client_List(boolean reset) {
  Serial.printf("New Station Number of connections : %i\n", WiFi.softAPgetStationNum());
  int ipidx = 0;

  // Special case for this AP enabled with DHT always stored in location 0
  clients[ipidx].ipAddress = WiFi.softAPIP();
  if(reset){ clients[ipidx].isAlive = true;}
  Serial.print(ipidx); Serial.print(":\t"); Serial.println(clients[ipidx].ipAddress.toString());
  ipidx++;  // ready for remote clients

  // Add the remote clients
  struct station_info *station_list = wifi_softap_get_station_info();
  while (station_list != NULL) {
    clients[ipidx].ipAddress = IPAddress((&station_list->ip)->addr);
    Serial.print(ipidx); Serial.print(":\t"); Serial.println(clients[ipidx].ipAddress.toString());
    if(reset){ clients[ipidx].isAlive = true;}  // Default to active until get_Data() is verified
if(reset){ Serial.print(ipidx); Serial.println(" = TRUE");}
    station_list = STAILQ_NEXT(station_list, next);
    ipidx++;    // ipidx++; is not working use long-hand
  }
  wifi_softap_free_station_info();
}



/*
 *
 * 				Update remote station values
 *
 */
boolean read_Client(int Station_Number){
	boolean retVal = false;
	Load_Client_List(false);	// Some reason the clist is not loaded on new client so update clist each call

// TODO remove after debug
Serial.print(F("Read Client ")); Serial.print(Station_Number);

if (clients[Station_Number].isAlive) {
		retVal = checkStation(&clients[Station_Number]);
		// TODO remove after debug
		Serial.println(F(" is Active "));
	} else {


// TODO remove after debug
Serial.println(F(" is Not Alive "));

	retVal = false;
	}

// TODO remove after debug
for(int s=0; s <= max_connection; s++){
	Serial.print(s);
	Serial.print(F("\tHostname: ")); Serial.print(clients[s].myHostName);
	Serial.print(F("\tIP Address: ")); Serial.print(clients[s].ipAddress);
	Serial.print(F("\tLast read: ")); Serial.println(clients[s].last_check);
}

	return retVal;
}

/**
 *
 *
 *
 *
 */
String SendHTML() {
  char tempFs[6];
  char humis[5];
  char connections[3];
  sprintf(connections, "%2d", WiFi.softAPgetStationNum());
  //String SendHTML(uint8_t led1stat,uint8_t led2stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head>\n";
  ptr += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>DigiTemp AP Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;} h6 {color: #DFF2FB;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #1abc9c;}\n";
  ptr += ".button-on:active {background-color: #16a085;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "<script type=\"text/javascript\">setTimeout(\"location.reload()\", 45000);</script>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>ESP8266 DigiTemp Server</h1>\n";
  ptr += "<h3>Using Access Point(AP) Mode ";
  ptr += connections;
  ptr += " clients</h3>\n";


  // Loop starts at one clients[0] is special case for local DHT
  for (int ii = 0; ii  <= WiFi.softAPgetStationNum(); ii++) { //max_connection
    if (clients[ii].isAlive) {
      ptr += "<p>"; // if(0 != ii) ptr += "Connection: ";
      ptr += clients[ii].myHostName;
      ptr += "<br>";
      ptr += "<a href=//";
      ptr += clients[ii].ipAddress.toString();
      ptr += "/getData";
      ptr += ">";
      ptr += clients[ii].ipAddress.toString();
      ptr += "  {{HighLow}}  ";
      ptr += "</a>";
      //      if(clients[ii].ipAddress ==  WiFi.softAPIP()){
      //        checkLocalStation(&clients[ii]);
      //      } else {
      //        checkStation(&clients[ii]);
      //      }
//     if (checkStation(&clients[ii])) {
//     if(clients[ii].isAlive) {
        if (isnan(clients[ii].Th_t.f)) {
          Serial.print(clients[ii].Th_t.f); Serial.println(" tempF is not a number");
        } else {
          Serial.print(clients[ii].Th_t.f);
        }
        if (isnan(clients[ii].Th_t.h)) {
          Serial.println("humi is not a number");
        }
        sprintf(tempFs, "%02.2f", clients[ii].Th_t.f);
        sprintf(humis, "%02.1f", clients[ii].Th_t.h);

        ptr += "<a class=\"button button-off\" href=\"/stations\"> ";
        ptr += tempFs;
        ptr += "F ";
        ptr += humis;
        ptr += "%</a>";
        char highlow[40];
        sprintf(highlow, "High: %02.2fF\t\tLow: %02.2fF", clients[ii].Th_t.tmax, clients[ii].Th_t.tmin);
        ptr.replace("{{HighLow}}", String(highlow));
      }
//      else {
//        ptr += "<a class=\"button button-off\" href=\"/stations\">Off Line</a>";
//        clients[ii].isAlive = false;
//      }
  } // for WiFi.softAPgetStationNum
  ptr += "<h6>"; ptr += copyrite; ptr += " "; ptr += compiledate; ptr += "</h6>";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}


/* Just a little test message.  Go to http://192.168.4.1 in a web browser
   connected to this access point to see it.
*/
void rootPage(){
//void handleRoot() {
  if(DEBUG) Serial.println("Hello from handleRoot");
  Server.send(200, "text/html", SendHTML());
	//Server.client().stop();
  Server.client().flush();
}


String pleaseWait() {
  //newClient = true;
  char connections[3];
  sprintf(connections, "%2d", WiFi.softAPgetStationNum());
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head>\n";
  ptr += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<script type=\"text/javascript\">setTimeout(\"location.replace(location.origin)\", 5000);</script>\n";
  ptr += "<title>DigiTemp AP Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Please Wait</h1>\n";
  ptr += "<h3>Loading ";
  ptr += connections;
  ptr += " clients</h3>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}


// Currently for Debug easy way to Reset client list from web page
void updateStations() {
  if(DEBUG) Serial.println("Hello from updateStations");
  Server.send(200, "text/html", pleaseWait());
  Server.client().flush();
  newClient = true; //clientList();
  //handleRoot();
}

void setupServer() {
  Server.on("/stations", updateStations);
  Serial.println("HTTP server started");
}
/*
                                                            Setup mDNS responder
                    Allows this device to be found on local network by using {nyhostname}.local instead of IP address
                    using an mDNS Client or Bonjour

*/
void  setupmDNS() {
  // Set up mDNS responder:
  if (!MDNS.begin(hostName)) {
    Serial.println(F("Error setting up MDNS responder!"));
  }
  Serial.print(F("mDNS responder started connect using http://"));
  Serial.print(hostName);
  Serial.println(F(".local"));
}

void my_setup() {
  hostName = MY_HOSTNAME;
  Serial.print(F("Running as Host "));
  Serial.println(hostName);
  setupAP();
  setupServer();
  setupmDNS();
}


/*
 *      Main loop
 *
 */
int client_num = 0;
void my_loop() {
  MDNS.update();    // required for the mDNS lookup this is the key to making mDNS work
  	  	  	  	  	// Undocumented I think update() is what responds to DNS requests
  	  	  	  	  	// and needs to be called frequently
  if(newClient){
	  if(DEBUG) Serial.println("There is a new client");
	  Load_Client_List(true);
	  newClient = false;
  }
  if(timeElapsed(minuteInterval/4)){
	  if(client_num > WiFi.softAPgetStationNum()) client_num = 0;
	  read_Client(client_num++);
  }
}

void do_serial(char r){
    Serial.print(r);
    if(r == '?') Load_Client_List(true);
    Serial.print(" Num connections: "); Serial.println(WiFi.softAPgetStationNum());
    Serial.print(" ");
    Serial.println(WiFi.localIP().toString());
    Serial.println(WiFi.softAPIP().toString());

    for(int s=0; s <= max_connection; s++){
      Serial.print(s);
      Serial.print(F("\tHostname: ")); Serial.print(clients[s].myHostName);
      Serial.print(F("\tIP Address: ")); Serial.println(clients[s].ipAddress);
    }
}

#endif		// DIGITEMPSERVER_H
