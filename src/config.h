#ifndef _CONFIG_H_
#define _CONFIG_H_

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
#define TEMP_ALARM_RANGE_LOWER 72
#define TEMP_ALARM_RANGE_UPPER 82
#define PRESSURE_ALARM_RANGE_LOWER 930UL
#define PRESSURE_ALARM_RANGE_UPPER 1020UL
#define HUMIDITY_ALARM_RANGE_LOWER 30
#define HUMIDITY_ALARM_RANGE_UPPER 60

#define buzzerPin 4

#endif // _CONFIG_H_