#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include "packet_t.h"
#include "packet_list.h"
#include "ui.h"

String serverName = "http://www.totalcareprog.com/cgi-bin/butchmonitor_save.php";

// called during setup(), and when a connection fails in loop()
void connectToWifi() {
   uint8_t attempts = 0;
   Serial.print("Connecting to wifi ");
   WiFi.begin(SSID, PASSWORD);
   while(WiFi.status() != WL_CONNECTED && attempts < 10) {
     Serial.print(". ");
     delay(500);
     attempts++;
   }
   Serial.println("");
   if (WiFi.status() != WL_CONNECTED) {
     Serial.println("Not connected to wifi");
     delay(500);
   }
   else {
     Serial.println("Connected to " + WiFi.localIP().toString());
   }
}

// BAS: if I start sending data to Arduino IO, this will go away.
// But I'd like to keep it during the transition.
// But no need to make it into, or include it in, a class, since it's going to go away.
bool transmitToWeb(){
    Serial.println("Transmitting to Jim's website");
    bool success = false;
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("wifi not connected");
      connectToWifi();
    }

    else {
      Serial.println("Connected to wifi");
      HTTPClient http;

      String serverPath = serverName 
      + "?battery1=" + String(battery1, 2) 
      + "&battery2=" + String(battery2, 2)
      + "&battery3=" + String(battery3)
      + "&temp=" + String(yourTemp)
      + "&humidity=" + String(yourHumidity)
      + "&pressure=" + String(yourPressure);

      //Serial.println(serverPath);
      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());
      
      // Send HTTP GET request
     //Serial.println("Upating the web");
        int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        //String payload = http.getString();
        Serial.println(http.getString());
        success = true;
      }else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
     
      // Free resources
      http.end();
    }
    turnOnLed(); // both LED's
    delay(1000);
    turnOFFLed(); // both LED's
    return success;
 
} // end transmit to web

#endif // #define _FUNCTIONS_H_
