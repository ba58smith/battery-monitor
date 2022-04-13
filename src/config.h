#ifndef _CONFIG_H_
#define _CONFIG_H_

// Network must be the same (14) for all of my units to communicate w/ each other.
#define LORA_NETWORK_ID 14

// Set the LoRa address of this ESP32 (the receiver)
// BAS: used only if the receiver starts to also start transmitting data
#define LORA_BASE_STATION_ADDRESS 10

// Define the range of the transmitter addresses that are valid for our project.
// Any packet received from a transmitter whose address is outside this range will be ignored.
// Addresses can be 0 to 65535
#define ADDRESS_RANGE_LOWER 49000
#define ADDRESS_RANGE_UPPER 49020

// Un-comment and change the baud rate below to change it.
// #define LORA_BAUD_RATE 115200     // default 115200

// Put your wifi credentials here
const char *SSID = "KeyAlmostWest";
const char *PASSWORD = "sfaesfae";

#define WEB_UPDATE_ALARM_AGE 3600000 // 1 hour

// BME280 alarm ranges
#define TEMP_ALARM_RANGE_LOWER 72
#define TEMP_ALARM_RANGE_UPPER 82
#define PRESSURE_ALARM_RANGE_LOWER 930
#define PRESSURE_ALARM_RANGE_UPPER 1020
#define HUMIDITY_ALARM_RANGE_LOWER 30
#define HUMIDITY_ALARM_RANGE_UPPER 60

#define buzzerPin 4

#endif // _CONFIG_H_