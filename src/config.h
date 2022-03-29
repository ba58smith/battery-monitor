// Network must be the same (2) for all of my units to communicate w/ each other.
// After re-writing the receiver code, this will change to some other unique identifier.
#define LORA_NETWORK_ID 2

// Set the LoRa address of this ESP32 (the receiver)
#define LORA_BASE_STATION_ADDRESS 10

// Un-comment any setting below you want to change
// #define LORA_FREQUENCY 915000000  // default 915000000
// #define LORA_BAUD_RATE 115200     // default 115200
// #define LORA_OUTPUT_POWER 15      // default 15
// #define LORA_SPREAD_FACTOR 12     // default 12
// #define LORA_BANDWIDTH 7          // default 7
// #define LORA_CODING_RATE 1        // default 1
// #define LORA_PREAMBLE 4           // default 4
