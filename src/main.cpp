// Base station (receiver) code

#include <Arduino.h>

#include "functions.h"

unsigned long lastWebUpdate = 0; 
unsigned long mainLoopDelay = 500;
unsigned long mainLoopCycle=0;
bool firstTime = true;
unsigned long webUpdateDelay = 600000; // delay 10 minutes


//++++++++++++++++++++++++++++++  SETUP +++++++++++++++++++++++++++++++++++++
void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  pinMode(26,OUTPUT); // this is for the big blue LED
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    while (1); // Don't proceed, loop forever
  }
  
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.display();
  delay(2000);

  // Draw a single pixel in white (dont recall why i need this, but when i took it out it broke)
  display.setTextSize(1);
  display.drawPixel(0, 0, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print(F("Connecting to: "));
  display.setCursor(0,20);
  display.setTextSize(2);
  display.println(F(SSID));
  display.display();
  display.setTextSize(1);
  
  connectToWifi();

  Serial2.print("AT\r\n");
  setLoRaParameters(); // sets up all the lora settings
  delay(500);
} // setup()


void loop()
{
  currentTime = millis();
  if (currentTime > mainLoopDelay + mainLoopCycle || firstTime == true)
  {
    mainLoopCycle = currentTime;
    firstTime = false;
    //Serial.println("looping...");
    checkformessages();
    if (currentTime > lastWebUpdate + webUpdateDelay)
    {
      transmitToWeb(lastWebUpdate);
      lastWebUpdate = currentTime;
    }
    getEnvironmental();
    if (WiFi.status() != WL_CONNECTED)
    {
      connectToWifi();
    }
  }
} // loop()

