// Base station (receiver) code

#include <Arduino.h>

#include "config.h"
#include "functions.h"
#include "reyax_lora.h"
#include "ui.h"
#include "packet_list.h"
#include "elapsedMillis.h"

// If you change the NETWORK_ID or BASE_STATION_ADDRESS (in config.h):
// Un-comment "#define LORA_SETUP_REQUIRED", upload and run once, then
// comment out "#define LORA_SETUP_REQUIRED".
// That will prevent writing the NETWORK_ID and BASE_STATION_ADDRESS to EEPROM every run.
//#define LORA_SETUP_REQUIRED
 
uint64_t loop_delay = 500;
uint64_t web_update_delay = 600000;    // every 10 minutes (600000)
uint64_t bme280_update_delay = 600000; // every 10 minutes
uint64_t packet_display_interval = 5000; // every 5 seconds
uint64_t last_web_update = millis(); // to avoid an alarm until the first one is sent

elapsedMillis loop_timer;
elapsedMillis web_update_timer;
elapsedMillis bme280_timer;
elapsedMillis packet_display_timer;

auto* lora = new ReyaxLoRa();

auto* ui = new UI();

auto* packet_list = new PacketList();

void setup() {
  // For Serial Monitor display of debug messages
  Serial.begin(115200);
  // Wait for the serial connection
  while (!Serial);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  lora->initialize();

#ifdef LORA_SETUP_REQUIRED
  lora->one_time_setup();
#endif

  // Add the appropriate "set" method(s) here to change most of
  // the LoRa parameters, if desired.
  // EXAMPLE: lora.set_output_power(10);

  ui->prepare_display();

  // Connect to wifi
  ui->before_connect_to_wifi_screen();
  connectToWifi(); //BAS: replace this with AdafruitIO::connect(). I need to call that command at some point,
  // and the first thing it does is disconnect from wifi, then re-connect.
  ui->after_connect_to_wifi_screen(WiFi.localIP().toString());

  //packet_list->make_fake_packets(); // BAS: remove this when you have real data coming from transmitters

} // setup()

void loop() {
  
  if (loop_timer > loop_delay) { // won't do anything until it runs the first time for loop_delay ms - that's OK.
    
    // check for new data on Serial2
    packet_list->get_new_packets();

    // read current bme280 data and display it
    if (bme280_timer > bme280_update_delay) {
      packet_list->update_BME280_packets();
      bme280_timer = 0;
    }

    if (web_update_timer > web_update_delay) {
      if (transmitToWeb()) {
        last_web_update = millis();
        packet_list->update_web_update_packet(last_web_update);
      }
      web_update_timer = 0;
    }

    if (packet_display_timer > packet_display_interval) {
      uint8_t list_size = packet_list->get_packet_list_size();
      if (list_size) { // there are packets to display
        ui->display_one_packet(packet_list->advance_one_packet());
        packet_display_timer = 0;
      }
    }
    
    loop_timer = 0;
  }
} // loop()

