#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_BME280.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 5
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// the line below is just a quick way to adjust the vertical seperation between the 
// text lines on the oled. So, below, line1 will start on y 17
#define line1 17
#define line2 32  // 20
#define line3 47  // 40
//#define line4 55

#define buzzer  4
#define VOLTAGE_ALARM_LEVEL 12.2

String battery1="";
String battery2="";
String battery3="";

unsigned long currentTime;
unsigned long lastBatt1update = 0;
unsigned long lastBatt2update = 0;
unsigned long lastBatt3update = 0;

String myRSSI;
char myradio;
String serverName = "http://www.totalcareprog.com/cgi-bin/butchmonitor_save.php";

int yourTemp = 80;
int yourPressure = 1022;
int yourHumidity = 50;

const char *SSID = "KeyAlmostWest";
const char *PASSWORD = "sfaesfae";

String SignalNoise;

Adafruit_BME280 bme; // I2C

// called during setup(), and when a connection fails in loop()
void connectToWifi(){
   WiFi.begin(SSID, PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
     delay(500);
  }
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("Connected to"));
  display.setCursor(0,30);
  display.setTextSize(1);
  display.println(WiFi.localIP());
  display.display();
  delay(5000);
  display.setTextSize(1);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("Awaiting data"));
  display.display();
}

// clears the info message at TOP of OLED
void clearComments(){
for (int y=0; y<=15; y++){
    for (int x=0; x<127; x++){
      display.drawPixel(x, y, BLACK); 
    }
    display.display();
  }
}

void showTemp(){
  clearComments();
  display.setCursor(0, 5);
  //display.println(F("show temps!"));
  display.print(F("T:"));
  display.setCursor(14, 5);
  display.print(yourTemp);
  
  display.setCursor(40, 5);
  display.print(F("H: "));
  display.setCursor(54, 5);
  display.print(yourHumidity);

  display.setCursor(80, 5);
  display.print(F("P: "));
  display.setCursor(94, 5);
  display.print(yourPressure);
  display.display(); // BS: this was missing from Jim's version
}

void successfulWebUpdate(){
  clearComments();
  display.setCursor(0, 5);
  display.println(F("Web Updated!"));
  //display.setCursor(100, 55);
  //display.println(myradio);
  display.display(); 
  delay(2000);
  showTemp();
}

void unSeccessfulWebUpdate(){
  clearComments();
  display.setCursor(0, 5);
  display.println(F("Web update Failed!"));
  //display.setCursor(100, 55);
  //display.println(myradio);
  display.display(); 
  delay(2000);
  showTemp();
}

void turnOnLed(){
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on
  digitalWrite(26,HIGH);
}

void turnOFFLed(){
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on
  digitalWrite(26,LOW); 
}

void soundAlarm(){
  for(int i = 0; i < 2; ++i){
    digitalWrite(buzzer, HIGH);
    delay(100);
    digitalWrite(buzzer, LOW);
    delay(300);
   }
  digitalWrite(buzzer, LOW);
}

// I don't think this is necessary, since soundAlarm() does what this does, at the end
void stopAlarm(){
    digitalWrite(buzzer, LOW);
}    

void checkLastUpdate(){
  //Serial.println("Checking last update");
  display.setTextSize(1);
  display.drawPixel(0, 0, SSD1306_WHITE);
  
    currentTime = millis();
    // last 3 update 
    if(currentTime > lastBatt3update + 400000 || lastBatt3update == 0){
     for (int y=line3; y<=line3+7; y++){
       for (int x=118; x<127; x++){
       display.drawPixel(x, y, BLACK); 
      }
    }
      //Serial.println("Long time!");
      display.setCursor(118, line3);
      display.println(F("?"));
      display.display();
    }else{
      display.setTextSize(1);
      display.setCursor(118, line3);
      display.println(F("*"));
      display.display();
    }
    // last 2 update
    if(currentTime > lastBatt2update + 400000 || lastBatt2update == 0){
     for (int y=line2; y<=line2+7; y++){
       for (int x=118; x<127; x++){
       display.drawPixel(x, y, BLACK); 
      }
    }
     // Serial.println("Long time!");
      display.setCursor(118, line2);
      display.println(F("?"));
      display.display();
    }else{
      display.setTextSize(1);
      display.setCursor(118, line2);
      display.println(F("*"));
      display.display();
    }
    if(currentTime > lastBatt1update + 400000 || lastBatt1update == 0){
      for (int y=line1; y<=line1+7; y++){
        for (int x=118; x<127; x++){
         display.drawPixel(x, y, BLACK); 
        }
      }
      
      display.setCursor(118, line1);
      display.println(F("?"));
      display.display();
    }else{
      
      display.setCursor(118, line1);
      display.println(F("*"));
      display.display();
    }
   // display.display();
}




// called by function checkformessages()
// if there is data, this function is called
void updateOLED1(){
  //Serial.println("battery 1");
  display.setTextSize(1);
  //display.drawPixel(0, line1, SSD1306_WHITE);
  lastBatt1update = millis();
  for (int y=line1; y<=line1+6; y++){
    for (int x=0; x<127; x++){
      display.drawPixel(x, y, BLACK); 
    }
  }
  
  display.setCursor(0, line1);
  display.println(F("Bes1:"));
  display.setCursor(30,line1);
  display.println(battery1);
  display.setCursor(65,line1);
  display.println(F("RSSI: "));
  display.setCursor(95,line1);
  display.println(myRSSI);
  display.display();      // Show initial text
  if(battery1.toFloat() < VOLTAGE_ALARM_LEVEL){
    soundAlarm();
  }else{
    stopAlarm();
  }
  showTemp();
}
// called by function checkformessages()
// if there is data, this function is called
void updateOLED2(){
  //Serial.println("battery 2");
  lastBatt2update = millis();
  //display.clearDisplay();
  for (int y=line2; y<=line2+6;y++){
    for (int x=0; x<127; x++){
      display.drawPixel(x, y, BLACK); 
    }
  }
  display.setCursor(0, line2);
  display.println(F("Bes2:"));
  display.setCursor(30,line2);
  display.println(battery2);
  display.setCursor(65,line2);
  display.println(F("RSSI: "));
  display.setCursor(95,line2);
  display.println(myRSSI);
  display.display();      // Show initial text
   delay(2000);
  if(battery2.toFloat() < VOLTAGE_ALARM_LEVEL){
    soundAlarm();
  }else{
    stopAlarm();
  }
  showTemp();
}

void updateOLED3(){
  //Serial.println("battery 3");
  lastBatt3update = millis();
  //display.clearDisplay();
  for (int y=line3; y<=line3+6; y++){
    for (int x=0; x<127; x++){
      display.drawPixel(x, y, BLACK); 
    }
  }
  display.setCursor(0, line3);
  display.println(F("Boat"));
  display.setCursor(30,line3);
  display.println(battery3);
  display.setCursor(65,line3);
  display.println(F("RSSI: "));
  display.setCursor(95,line3);
  display.println(myRSSI);
  display.display();      // Show initial text
   delay(100);
  if(battery3.toFloat() < VOLTAGE_ALARM_LEVEL){
    soundAlarm();
  }else{
    stopAlarm();
  }
  showTemp();
}

void getMyVoltage(){
    Serial2.read(); // skip the comma after <Address>
    Serial2.read(); // advances past the <Length>, which we expect to always be 9 or less. If 10 or more, this won't work.
    Serial2.read(); // skip the comma after <Length>
    char network = Serial2.read(); // this is not reading the networkID - it's reading the first character of <Data>.
    //Serial.print("network: ");
    //Serial.println(network);

    if(network == '2'){ // this will always be true for a packet received from one of "OUR" transmitters, because that's what's being
                        // sent by the transmitters as the first character of <Data>.
      turnOnLed();
      clearComments();
      display.setCursor(0, 5);
      display.println(F("Receiving data!"));
      display.setCursor(100, 5);
      display.println(myradio);
      Serial2.read(); // skip the comma after the "2"
      // myradio was extracted from Serial2 prior to this, in checkformessages()
      // assign the value in <Data> to the appropriate batteryx variable
      if(myradio == '1'){
        battery1 = Serial2.readStringUntil(','); // reads characters 3-7 of <Data>
      }else if(myradio == '2'){
        battery2 = Serial2.readStringUntil(','); // reads characters 3-7 of <Data>
      }else if(myradio == '3'){
        battery3 = Serial2.readStringUntil(','); // reads characters 3-7 of <Data>
      }
     
      Serial2.read(); // skip the comma after <Data>
      myRSSI = Serial2.readStringUntil(',');
      //Serial.println(myRSSI);
      Serial2.read(); // skip the comma after <RSSI>
      SignalNoise = Serial2.readStringUntil('\n'); // this isn't used anywhere
      if(myradio == '1'){
       updateOLED1();
      }else if(myradio == '2'){
       updateOLED2();
      }else if(myradio == '3'){
        updateOLED3();
      }
      
      network = 0;
    }
}

void checkformessages()
{
  if (Serial2.available() > 0)
  {
    while (Serial2.available() > 0)
    {
      //Serial.println(Serial2.readStringUntil('\n'));
      if (Serial2.read() == '+')
      {
        if (Serial2.read() == 'R')
        {
          if (Serial2.read() == 'C')
          {
            if (Serial2.read() == 'V')
            {
              if (Serial2.read() == '=')
              {
                // The first character of every received payload is the transmitter identifier, known here as myradio
                myradio = Serial2.read();
                // i now know what radio transmitted, so lets get busy parsing the rest of the string
                getMyVoltage();
              }
            }
          }
        }
      }
      // check to see when the last update was
      checkLastUpdate();
    }
    delay(1000);                    // let the comments stay viewable for a bit
    digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
    // clear the receiving data text
    //clearComments();
    // end the clear the receiving data text

    turnOFFLed();
  }
} // end of check for messages

// only called to see if the radio is ok.. not apart of the check for incoming messages from boats
// This just checks to see if there is ANY reply from the transmitter. It's not checking to see
// that the reply is "OK", which is what it's supposed to be.
void checkincoming()
{
  String response = "";
  if (Serial2.available() > 0)
  {
    delay(20);
    Serial.println("incoming!");
    while (Serial2.available() > 0)
    {
      char c = Serial2.read();
      response += c;
      //Serial.print(c);
    }
    if (response != "")
    {
      Serial.println("got a response: ");
      Serial.println(response);
      response = "";
      delay(1000);
    }
    else
    {
      Serial.println("no response");
    }
  } // checkIncoming
}
// called only in setup()
void setLoRaParameters(){
  Serial.print("Get address:");
  Serial2.print("AT+ADDRESS=10\r\n");
    delay(200);
  Serial2.print("AT+NETWORKID=2\r\n");
  delay(200);
  Serial2.print("AT+ADDRESS?\r\n");
  delay(200);
  checkincoming();
  Serial2.print("AT+IPR=115200\r\n");
   delay(200);
   checkincoming();
  Serial2.print("AT+BAND?\r\n");
  checkincoming();
  delay(1000);
  Serial2.print("AT+PARAMETER=12,7,1,4\r\n");
   delay(200);
    checkincoming();
  Serial2.print("AT+PARAMETER?\r\n");
  //Serial.print("AT+PARAMETER?\r\n");
  delay(1000);
   checkincoming();
  Serial2.print("AT+CRFOP=15\r\n");
   delay(200);
   checkincoming();
  //Serial2.print("AT+CRFOP?\r\n");
  //Serial.print("AT+CRFOP?\r\n");
}

void transmitToWeb(unsigned long& lastUpdate){
    
    if(WiFi.status()== WL_CONNECTED){
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
        String payload = http.getString();
        Serial.println(payload);
        display.setCursor(0, 5);
        successfulWebUpdate();
      }else {
        Serial.print("Error code: ");
        display.setCursor(0, 5);
        Serial.println(httpResponseCode);
        unSeccessfulWebUpdate();
      }
     
      // Free resources
      http.end();
    }else {
      // well, lets get connected!
      Serial.println("WiFi Disconnected");
      connectToWifi();
    }
    //digitalWrite(buzzer, HIGH);
    turnOnLed();
    delay(1000);
    //digitalWrite(buzzer, LOW);
    turnOFFLed();
    lastUpdate = millis();
  //}
  
 
} // end transmit to web

// bme280 function
void getEnvironmental() {
 
  yourTemp = bme.readTemperature();
  yourTemp = (yourTemp  * 1.8) + 32;
  yourPressure = (bme.readPressure() / 100.0F);
  //Serial.println(" hPa");
  yourHumidity = bme.readHumidity();
 
}

#endif // #define _FUNCTIONS_H_
