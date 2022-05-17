#ifndef _SECRET_CONFIG_H_
#define _SECRET_CONFIG_H_

// Network must be the same for all of your units to communicate w/ each other.
// Valid network ID's are 1 - 15: DO NOT USE network ID 0.
#define LORA_NETWORK_ID 15

// Set the address of the physical LoRa connected to this ESP32
// Every LoRa unit must have a unique address on your network.
// Valid addresses are 0 - 65535. (Define a valid range for YOUR
// network below.) Format is xxxUL, to define the address as unsigned long.
#define LORA_NODE_ADDRESS 65000UL

// Set the LoRa address of this ESP32 (the receiver). Same as LORA_NODE_ADDRESS
#define LORA_BASE_STATION_ADDRESS 65000UL

// Define the range of the transmitter addresses that are valid for your project.
// Any packet received from a transmitter whose address is outside this range will be ignored.
// Addresses can be 0 to 65535. Format is xxUL
#define ADDRESS_RANGE_LOWER 65000UL // Base station - see above
#define ADDRESS_RANGE_UPPER 65100UL

// Put your wifi credentials here
const char *SSID = "YourWifiSSIDHere";
const char *PASSWORD = "YourWifiPasswordHere";

#define INFLUXDB_URL "YourServerAddressAndPortHere" // format: "http://server-name.com:8086"
// InfluxDB v1 database name 
#define INFLUXDB_DB_NAME "YourDBNameHere"
#define INFLUXDB_USER "YourDBUserHere"
#define INFLUXDB_PASSWORD "YourDBPasswordHere"

#endif // _SECRET_CONFIG_H_