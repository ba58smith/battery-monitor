#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "packet_list.h"
#include "ui.h"

// BAS: get rid of these
String battery1="11.11";
String battery2="12.22";
String battery3="13.33";

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
}

// BAS: if I start sending data to Arduino IO, this will go away.
// But I'd like to keep it during the transition.
// But no need to make it into, or include it in, a class, since it's going to go away.
bool transmitToWeb(){
    bool success = false;
    
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi Disconnected");
      connectToWifi();
    }

    else {  
      HTTPClient http;

      String serverPath = serverName 
      + "?battery1=" + battery1.toFloat() 
      + "&battery2=" + battery2.toFloat() 
      + "&battery3=" + battery3.toFloat()
      + "&temp=" + yourTemp
      + "&humidity=" + yourHumidity
      + "&pressure=" + yourPressure;

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
        // BAS: display.setCursor(0, 5);
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
