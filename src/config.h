#ifndef _CONFIG_H_
#define _CONFIG_H_

// BAS: these can go when I stop sending to Jim's website
uint16_t yourTemp = 80;
float yourPressure = 29.92;
uint16_t yourHumidity = 50;

#define BASE_STATION

// Network must be the same (14) for all of my units to communicate w/ each other.
#define LORA_NETWORK_ID 14

// Set the address of the physical LoRa connected to this ESP32
#define LORA_NODE_ADDRESS 2200UL

// Set the LoRa address of this ESP32 (the receiver)
#define LORA_BASE_STATION_ADDRESS 2200UL

// Define the range of the transmitter addresses that are valid for our project.
// Any packet received from a transmitter whose address is outside this range will be ignored.
// Addresses can be 0 to 65535
#define ADDRESS_RANGE_LOWER 2200UL // Base station
#define ADDRESS_RANGE_UPPER 2240UL

// Un-comment and change the baud rate below to change it.
// #define LORA_BAUD_RATE 115200ULL     // default 115200

// Put your wifi credentials here
const char *SSID = "KeyAlmostWest";
const char *PASSWORD = "sfaesfae";

#define WEB_UPDATE_ALARM_AGE 3600000ULL // 1 hour

// BME280 alarm ranges
#define TEMP_ALARM_RANGE_LOWER 72.0F // s/b 72.0
#define TEMP_ALARM_RANGE_UPPER 85.0F // s/b 85
#define PRESSURE_ALARM_RANGE_LOWER 29.0F // inches of hg - normal is 29.0 - 31.0
#define PRESSURE_ALARM_RANGE_UPPER 31.0F // s/b 31.0
#define HUMIDITY_ALARM_RANGE_LOWER 30.0F // s/b 30.0 - 50.0 per Mayo Clinic
#define HUMIDITY_ALARM_RANGE_UPPER 50.0F // 50.0 gave too many false alarms

#endif // _CONFIG_H_