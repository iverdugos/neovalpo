/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-relay-module-ac-web-server/
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/
/*moded by Ivan Verdugo for Neovalpo Festival, Chile. This version
 * turns on a 30A relay which turns on an electric water boiler
 */


// Import required libraries
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h> 
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"

// Set to true to define Relay as Normally Open (NO)
#define RELAY_NO    false

// Set number of relays
#define NUM_RELAYS  3

// Assign each GPIO to a relay
int relayGPIOs[NUM_RELAYS] = {2, 4, 12};
//power 1 = gpo 2 = d4
//power 2=  gpo 4 = d2
//power 3 = gpo 14 = d5
//power 4 = gpo 12 = d6
//Power 5 = gpo 13 = d7
//Power 6 = gpo 0 = d3

char* ssid = "WifiTelsur_RDS";
char* password = "70392526";


const char *softAP_ssid = "DOMO-Relay";
const char *softAP_password = "12345678";

const char *myHostname = "esp8266";

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
//char ssid[32] = "";
//char password[32] = "";



const char* PARAM_INPUT_1 = "relay";  
const char* PARAM_INPUT_2 = "state";

DNSServer dnsServer;
const byte DNS_PORT = 53;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

/** Should I connect to WLAN asap? */
boolean connect;
long lastConnectTry = 0;

/** Current WLAN status */
int status = WL_IDLE_STATUS;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Consolas; display: inline-block; text-align: center;}
    h4 { color: white;}
    h2 {font-size: 3.0rem; color: white;}
    p {font-size: 3.0rem; color: white;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: DarkViolet; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: white; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: LawnGreen}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
<body style="background-color:black;">
  <h2>Hervidor</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
  xhr.send();
}</script>
  <p></p>
  //<p>Instructions:</p>
  //<p>Some instructions</p> 
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for(int i=1; i<=NUM_RELAYS; i++){
      String relayStateValue = relayState(i);
      buttons+= "<h4>Switch #" + String(i) + " - GPIO " + relayGPIOs[i-1] + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" "+ relayStateValue +"><span class=\"slider\"></span></label>";
    }
    return buttons;
  }
  return String();
}

String relayState(int numRelay){
  if(RELAY_NO){
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "";
    }
    else {
      return "checked";
    }
  }
  else {
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "checked";
    }
    else {
      return "";
    }
  }
  return "";
}

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }
  void handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html>");
    response->print("<head>");
    response->print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    response->print("<style>");
    response->print("html {font-family: Consolas; display: inline-block; text-align: center;}");
    response->print("tr {color: white}");
    response->print("td {color: white}");
    response->print("h2 {font-size: 3.0rem; color: white;}");
    response->print("h4 { color: white;}");
    response->print("p {font-size: 2.0rem;color: white;}");
    response->print("body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}");
    response->print(".switch {position: relative; display: inline-block; width: 120px; height: 68px}");
    response->print(".switch input {display: none}");
    response->print(".slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: DarkViolet; border-radius: 34px}");
    response->print(".slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: white; -webkit-transition: .4s; transition: .4s; border-radius: 68px}");
    response->print("input:checked+.slider {background-color: DarkOrchid}");
    response->print("input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}");
    response->print("</style>");
    response->print("<title>Portal Cautivo</title>");
    response->print("</head><body>");
    response->print("<body style=\"background-color:black;\">");
    response->print("<p>Como conectarte a Wifi</p>");
    response->printf("<p>Tu IP es: http://%s%s</p>", request->host().c_str(), request->url().c_str());
    response->printf("<p>aca <a href='http://%s'>boton pa algo</a></p>", WiFi.softAPIP().toString().c_str());
    //response->printf("<p>You are connected through the soft AP: ") + softAP_ssid + "</p>");
    response->printf("<p>IP : %s </p>", WiFi.softAPIP().toString().c_str());
    response->printf("<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
    int n = WiFi.scanNetworks();
    if (n > 0) {
      for (int i = 0; i < n; i++) {
          response->printf("<p>lista de ssids : %i </p>", WiFi.scanNetworks(i));

      }
    } else {
     response->print("<tr><td>No se encuentran redes</td></tr>");
   }
    response->print("</table>");
    response->print("\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>");
    response->print("<input type='text' placeholder='network' name='n'/>");
    response->print("<br /><input type='password' placeholder='password' name='p'/>");
    response->print("<br /><input type='submit' value='Connect/Disconnect'/></form>");
    response->print("<p>You may want to <a href='/'>return to the home page</a>.</p>");
    response->print("</body></html>");
    request->send(response);
  }
};


void setup(){
  // Serial port for debugging purposes
  delay(1000);
  Serial.begin(115200);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  //WiFi.softAP(softAP_ssid, softAP_password);
  WiFi.softAP(softAP_ssid);
  delay(500);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  //more handlers...

  
  // Set all relays to off when the program starts - if set to Normally Open (NO), the relay is off when you set the relay to HIGH
  for(int i=1; i<=NUM_RELAYS; i++){
    pinMode(relayGPIOs[i-1], OUTPUT);
    if(RELAY_NO){
      digitalWrite(relayGPIOs[i-1], HIGH);
    }
    else{
      digitalWrite(relayGPIOs[i-1], LOW);
    }
  }
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  /*
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.on("/generate_204", handleRoot);
  server.on("/fwlink", handleRoot);
  server.onNotFound ( handleNotFound );

*/
  
  // Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      if(RELAY_NO){
        Serial.print("NO ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], !inputMessage2.toInt());
      }
      else{
        Serial.print("NC ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], inputMessage2.toInt());
      }
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage + inputMessage2);
    request->send(200, "text/plain", "OK");
  });
  server.begin();
  Serial.println("HTTP server started");
  //loadCredentials();  // Load WLAN credentials from network
  connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
}

void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin ( ssid, password );
  int connRes = WiFi.waitForConnectResult();
  Serial.print ( "connRes: " );
  Serial.println ( connRes );
}
  
void loop() {
    if (connect) {
    Serial.println ( "Connect requested" );
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  {
    int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000) ) {
      /* If WLAN disconnected and idle try to connect */
      /* Don't set retry time too low as retry interfere the softAP operation */
      connect = true;
    }
    if (status != s) { // WLAN status change
      Serial.print ( "Status: " );
      Serial.println ( s );
      status = s;
      if (s == WL_CONNECTED) {
        /* Just connected to WLAN */
        Serial.println ( "" );
        Serial.print ( "Connected to " );
        Serial.println ( ssid );
        Serial.print ( "IP address: " );
        Serial.println ( WiFi.localIP() );

        // Setup MDNS responder
        if (!MDNS.begin(myHostname)) {
          Serial.println("Error setting up MDNS responder!");
        } else {
          Serial.println("mDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
        }
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
  }

  
  dnsServer.processNextRequest();

}
