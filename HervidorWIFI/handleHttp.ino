

void handleWifi() {
  if (WiFi.localIP() == apIP) {
    //response->print("<p>You are in the access point</p>");  
  } else {
   // response->print("<p>You are connected through the wifi network: ") + ssid + "</p>");
  }
}


void handleWifiSave() {
  Serial.println("wifi save");
//  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
//  server.arg("p").toCharArray(password, sizeof(password) - 1);
//  server.sendHeader("Location", "wifi", true);
//  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
//  server.sendHeader("Pragma", "no-cache");
//  server.sendHeader("Expires", "-1");
//   server.send ( 302, "text/plain", "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
//  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}


void handleNotFound() {
  //if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    //return;
  //}
  String message = "File Not Found\n\n";
  message += "URI: ";
  //message += server.uri();
  message += "\nMethod: ";
  //message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  //message += server.args();
  message += "\n";

  //for ( uint8_t i = 0; i < server.args(); i++ ) {
   // message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  //}
  //server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  //server.sendHeader("Pragma", "no-cache");
  //server.sendHeader("Expires", "-1");
  //server.send ( 404, "text/plain", message );
}
