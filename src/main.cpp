// Base station (receiver) code

#include <Arduino.h>
#include "config.h"
#include "packet_t.h"
#include "packet_list.h"
#include "reyax_lora.h"
#include "ui.h"
#include "queues.h"
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
 
uint64_t wifi_check_delay = 30000; // every 30 seconds
uint64_t bme280_update_delay = 600000; // every 10:00
bool first_run = true;
uint64_t packet_display_interval = 3500; // every 3.5 seconds
uint64_t alarm_email_delay = 300000;   // every 5:00
uint64_t sys_time_display_delay = 30000; // every 30 seconds

elapsedMillis wifi_check_timer;
elapsedMillis bme280_timer;
elapsedMillis packet_display_timer;
elapsedMillis alarm_email_timer;
elapsedMillis sys_time_display_timer;

//auto* lora = new ReyaxLoRa(); BAS

auto* ui = new UI(blue_led_pin, buzzer_pin);

//auto* net = new Internet(ui); BAS

//auto* bme280 = new Adafruit_BME280(); BAS

//auto* packet_list = new PacketList(ui, bme280); BAS

void setup() {
  // For Serial Monitor display of debug messages
  Serial.begin(115200);
  // Wait for the serial connection
  while (!Serial);

  //lora->initialize(); BAS

#ifdef LORA_SETUP_REQUIRED
  lora->one_time_setup();
#endif

  // Add the appropriate "set" method(s) here to change most of
  // the LoRa parameters, if desired.
  // EXAMPLE: lora.set_output_power(10);

  initialize_queues();
  //packet_list->start_bme280(); BAS
  //packet_list->start_tasks(); BAS
  //net->start_tasks(); BAS
  ui->prepare_display();

  // Connect to wifi
  //net->connect_to_wifi();

} // setup()

void loop() {
  /*
  // periodically make sure we're still connected to wifi
  if (wifi_check_timer > wifi_check_delay) {
    if (!net->connected_to_wifi()) {
      net->connect_to_wifi();
    }
  }
  
  // read current bme280 data and add its packets to the queue
  if (first_run || bme280_timer > bme280_update_delay) {
    packet_list->update_BME280_packets();
    bme280_timer = 0;
    first_run = false;
  }

  if (packet_display_timer > packet_display_interval) {
    if (packet_list->packet_list_not_empty()) {
      ui->display_one_packet(packet_list->advance_one_packet());
      packet_display_timer = 0;
    }
  }
  */

  if (sys_time_display_timer > sys_time_display_delay) {
    ui->display_system_time();
    sys_time_display_timer = 0;
  }  
/*
  if (alarm_email_timer > alarm_email_delay) {
    Packet_it_t it_begin = packet_list->get_packets_begin();
    Packet_it_t it_end = packet_list->get_packets_end();
    net->send_alarm_email(it_begin, it_end);
    alarm_email_timer = 0;
  }
  */

} // loop()
