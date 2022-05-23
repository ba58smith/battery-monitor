// Base station (receiver) code

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
// comment out "#define LORA_SETUP_REQUIRED" and upload.
// That will prevent writing the NETWORK_ID and BASE_STATION_ADDRESS to EEPROM every run.
//#define LORA_SETUP_REQUIRED

uint8_t blue_led_pin = 26;
uint8_t buzzer_pin = 4;
 
uint64_t packet_check_delay = 500;
//uint64_t web_update_delay = 660000;    // every 11:00
uint64_t influx_update_delay = 5000;   // every 5 seconds
uint64_t bme280_update_delay = 600000; // every 10:00
uint64_t alarm_email_delay = 300000;   // every 5:00
bool first_run = true;
uint64_t packet_display_interval = 3500; // every 3.5 seconds
//uint64_t last_web_update = millis(); // to avoid an alarm until the first one is sent

elapsedMillis packet_check_timer;
//elapsedMillis web_update_timer;
elapsedMillis bme280_timer;
elapsedMillis packet_display_timer;
elapsedMillis influx_update_timer;
elapsedMillis alarm_email_timer;

auto* lora = new ReyaxLoRa();

auto* ui = new UI(blue_led_pin, buzzer_pin);

auto* net = new Internet(ui);

auto* bme280 = new Adafruit_BME280();

auto* packet_list = new PacketList(ui, bme280);

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
  ui->before_connect_to_wifi_screen(net->get_ssid());
  net->connect_to_wifi();
  ui->after_connect_to_wifi_screen(net->connected_to_wifi(), net->get_ip());

  configTime(-18000, 3600, "pool.ntp.org"); // Connect to NTP server with -5 TZ offset (-18000), 1 hr DST offset (3600).

} // setup()

void loop() {
  
  if (packet_check_timer > packet_check_delay) {
    // check for new packets coming in from transmitters on Serial2
    packet_list->get_new_packets();
    packet_check_timer = 0;
  }

  // read current bme280 data and add/update its packets in the list
  if (first_run ||  bme280_timer > bme280_update_delay) {
    packet_list->update_BME280_packets();
    bme280_timer = 0;
    first_run = false;
  }

  /*
  if (web_update_timer > web_update_delay) {
    if (net->transmit_to_web()) {
      last_web_update = millis();
      packet_list->update_web_update_packet(last_web_update);
    }
    web_update_timer = 0;
  }
  */

  if (packet_display_timer > packet_display_interval) {
    if (packet_list->packet_list_not_empty()) {
      ui->display_one_packet(packet_list->advance_one_packet());
      packet_display_timer = 0;
    }
  }

  if (influx_update_timer > influx_update_delay) {
    Packet_it_t it_begin = packet_list->get_packets_begin();
    Packet_it_t it_end = packet_list->get_packets_end();
    net->send_packets_to_influx(it_begin, it_end);
    influx_update_timer = 0;
  }

  if (alarm_email_timer > alarm_email_delay) {
    Packet_it_t it_begin = packet_list->get_packets_begin();
    Packet_it_t it_end = packet_list->get_packets_end();
    net->send_alarm_email(it_begin, it_end);
    alarm_email_timer = 0;
  }
} // loop()
