// Base station (aka receiver, aka battery monitor) code

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
// BAS: change some of the LoRa settings to try to improve the connection. Too many failed attempts for some reason.
// A detailed study: https://www.researchgate.net/publication/317823578_Evaluation_of_the_Reliability_of_LoRa_Long-Range_Low-Power_Wireless_Communication
// Two combinations that worked really well in the study:
// SF=9, CR=4/5, BW=125kHz
// SF=12, CR=4/8, BW=500kHz
// Higher "spreading factor" improves sensitivity, slows down transmission
// Lower "bandwidth" improves sensitivity, slows transmission
// Higher "coding rate" improves sensitivity, slows transmission
// Higher "preamble" may improve reliability, slows transmission slightly


uint8_t buzzer_pin = 4;
uint8_t tilt_switch_pin = 13;
bool cancel_screensaver = false;
 
uint64_t wifi_check_delay = 30000; // every 30 seconds
uint64_t bme280_update_delay = 600000; // every 10:00
bool first_run = true;
uint64_t packet_display_interval = 3000; // every 3 seconds
uint64_t alarm_email_delay = 45000;   // every 45 seconds
uint64_t sys_time_display_delay = 30000; // every 30 seconds
uint64_t screensaver_delay = 90000; // after 90 seconds

elapsedMillis wifi_check_timer;
elapsedMillis bme280_timer;
elapsedMillis packet_display_timer;
elapsedMillis alarm_email_timer;
elapsedMillis sys_time_display_timer;

auto* lora = new ReyaxLoRa();

auto* ui = new UI(buzzer_pin);

auto* net = new Internet(ui);

auto* bme280 = new Adafruit_BME280();

auto* packet_list = new PacketList(ui, bme280);

// to wake up the display with the tilt switch
void IRAM_ATTR wakeup_isr() {
  cancel_screensaver = true;
}

void setup() {
  pinMode(tilt_switch_pin, INPUT_PULLDOWN);
  attachInterrupt(tilt_switch_pin, wakeup_isr, CHANGE);
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
  // EXAMPLE: lora->set_output_power(10);

  initialize_queues();
  packet_list->start_bme280();
  packet_list->start_tasks();
  net->start_tasks();
  ui->prepare_display();

  // Connect to wifi
  net->connect_to_wifi();

} // setup()

void loop() {
  
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

  if (sys_time_display_timer > sys_time_display_delay) {
    ui->display_system_time();
    sys_time_display_timer = 0;
  }  

  if (alarm_email_timer > alarm_email_delay) {
    Packet_it_t it_begin = packet_list->get_packets_begin();
    Packet_it_t it_end = packet_list->get_packets_end();
    net->send_alarm_emails(it_begin, it_end);
    alarm_email_timer = 0;
  }

  if (ui->screensaver_timer_ > screensaver_delay) {
    ui->screensaver(true);
    ui->screensaver_timer_ = 0;
  }

  if (ui->screensaver_is_on() && cancel_screensaver) {
    ui->screensaver(false);
    cancel_screensaver = false;
  }

} // loop()
