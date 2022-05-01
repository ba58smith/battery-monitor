// Base station (receiver) code

// BAS: get rid of these when no longer sending to Jim's website
float battery1 = 0.0; // Bess1
float battery2 = 0.0; // Bess2
float battery3 = 0.0; // Boat

#include <Arduino.h>
#include "config.h"
#include "packet_t.h"
#include "packet_list.h"
#include "reyax_lora.h"
#include "ui.h"

#include "internet.h"
#include "elapsedMillis.h"
#include <Adafruit_BME280.h>

// If you change the NETWORK_ID or BASE_STATION_ADDRESS (in config.h):
// Un-comment "#define LORA_SETUP_REQUIRED", upload and run once, then
// comment out "#define LORA_SETUP_REQUIRED".
// That will prevent writing the NETWORK_ID and BASE_STATION_ADDRESS to EEPROM every run.
//#define LORA_SETUP_REQUIRED

uint8_t blue_led_pin = 26;
uint8_t buzzer_pin = 4;
 
uint64_t packet_check_delay = 500;
uint64_t web_update_delay = 660000;    // every 11:00
uint64_t bme280_update_delay = 600000; // every 10:00
uint64_t packet_display_interval = 3500; // every 3.5 seconds
uint64_t last_web_update = millis(); // to avoid an alarm until the first one is sent

elapsedMillis packet_check_timer;
elapsedMillis web_update_timer;
elapsedMillis bme280_timer;
elapsedMillis packet_display_timer;

auto* lora = new ReyaxLoRa();

auto* ui = new UI(blue_led_pin, buzzer_pin);

auto* bme280 = new Adafruit_BME280();

auto* packet_list = new PacketList(ui, bme280);

auto* net = new Internet();

void setup() {
  // For Serial Monitor display of debug messages
  Serial.begin(115200);
  // Wait for the serial connection
  while (!Serial);

  lora->initialize();

#ifdef LORA_SETUP_REQUIRED
  lora->one_time_setup();
#endif

  // Add the appropriate "set" method(s) here to change most of
  // the LoRa parameters, if desired.
  // EXAMPLE: lora.set_output_power(10);

  packet_list->start_bme280();
  
  ui->prepare_display();

  // Connect to wifi
  ui->before_connect_to_wifi_screen();
  net->connect_to_wifi(); //BAS: replace this with AdafruitIO::connect(). I need to call that command at some point,
  // and the first thing it does is disconnect from wifi, then re-connect.
  ui->after_connect_to_wifi_screen(WiFi.localIP().toString());

} // setup()

void loop() {
  
  if (packet_check_timer > packet_check_delay) {
    // check for new packets coming in from transmitters on Serial2
    packet_list->get_new_packets();
    packet_check_timer = 0;
  }

  // read current bme280 data and add/update its packets in the list
  if (bme280_timer > bme280_update_delay) {
    packet_list->update_BME280_packets();
    bme280_timer = 0;
  }

  if (web_update_timer > web_update_delay) {
    // BAS: move LED and status line control inside transmit_to_web when Internet class gets a UI pointer
    ui->turnOnLed();
    ui->update_status_line("Transmitting to web");
    if (net->transmit_to_web()) {
      last_web_update = millis();
      packet_list->update_web_update_packet(last_web_update);
    }
    web_update_timer = 0;
    ui->turnOFFLed();
    ui->update_status_line("Waiting for data");
  }

  if (packet_display_timer > packet_display_interval) {
    if (packet_list->packet_list_not_empty()) {
      ui->display_one_packet(packet_list->advance_one_packet());
      packet_display_timer = 0;
    }
  }
} // loop()
