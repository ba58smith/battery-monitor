#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "secret_config.h"

// BAS: these can go when I stop sending to Jim's website
uint16_t yourTemp = 80;
float yourPressure = 29.92;
uint16_t yourHumidity = 50;

#define BASE_STATION

// Un-comment and change the baud rate below to change it.
// #define LORA_BAUD_RATE 115200ULL     // default 115200

#define WEB_UPDATE_ALARM_AGE 3600000ULL // 1 hour

// BME280 alarm ranges
#define TEMP_ALARM_RANGE_LOWER 72.0F // s/b 72.0
#define TEMP_ALARM_RANGE_UPPER 85.0F // s/b 85
#define PRESSURE_ALARM_RANGE_LOWER 29.0F // inches of hg - normal is 29.0 - 31.0
#define PRESSURE_ALARM_RANGE_UPPER 31.0F // s/b 31.0
#define HUMIDITY_ALARM_RANGE_LOWER 30.0F // s/b 30.0 - 50.0 per Mayo Clinic
#define HUMIDITY_ALARM_RANGE_UPPER 52.0F // 50.0 gave too many false alarms

#endif // _CONFIG_H_