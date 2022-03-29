// Base station (receiver) code

#include <Arduino.h>

#include "config.h"
#include "functions.h"
#include "reyax_lora.h"

// If you change the NETWORK_ID (below) or NODE_ADDRESS (in config.h):
// Un-comment "#define LORA_SETUP_REQUIRED", upload and run once, then comment out "#define LORA_SETUP_REQUIRED".
// That will prevent writing the NETWORK_ID and NODE_ADDRESS to EEPROM every run.
#define LORA_SETUP_REQUIRED

unsigned long lastWebUpdate = 0; 
unsigned long mainLoopDelay = 500;
unsigned long mainLoopCycle=0;
bool firstTime = true;
unsigned long webUpdateDelay = 600000; // delay 10 minutes

ReyaxLoRa lora(LORA_NETWORK_ID, LORA_BASE_STATION_ADDRESS);

void setup() {
  
  // For Serial Monitor display of debug messages
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  pinMode(blueLED,OUTPUT);

  lora.initialize();

#ifdef LORA_SETUP_REQUIRED
  lora.one_time_setup();
#endif

  // Use the appropriate "set" method(s) here to change most of
  // the LoRa parameters if desired.

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

  // Connect to wifi
  display.setCursor(0,0);
  display.print(F("Connecting to: "));
  display.setCursor(0,20);
  display.setTextSize(2);
  display.println(F(SSID));
  display.display();
  display.setTextSize(1);
  connectToWifi();

  
} // setup()


void loop()
{
  currentTime = millis();
  if (currentTime > mainLoopDelay + mainLoopCycle || firstTime == true)
  {
    mainLoopCycle = currentTime;
    firstTime = false;
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

