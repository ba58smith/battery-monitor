#ifndef _CONFIG_H_
#define _CONFIG_H_

// Comment out the "secret_config.h" line, and UN-comment
// the "default_config.h" line. Then edit the default_config.h
// file to include all of your sensitive information.
#include "secret_config.h"
// #include "default_config.h"

#define BASE_STATION

// Un-comment and change the baud rate below to change it.
// #define LORA_BAUD_RATE 115200ULL     // default 115200

#define WEB_UPDATE_ALARM_AGE 3600000ULL // 1 hour

// BME280 alarm ranges
#define TEMP_ALARM_RANGE_LOWER 72.0F // s/b 72.0
#define TEMP_ALARM_RANGE_UPPER 85.0F // s/b 85.0
#define TEMP_ALARM_EMAIL_THRESHOLD 60 // in MINUTES
#define PRESSURE_ALARM_RANGE_LOWER 29.0F // inches of hg - normal is 29.0 - 31.0
#define PRESSURE_ALARM_RANGE_UPPER 31.0F // s/b 31.0
#define PRESSURE_ALARM_EMAIL_THRESHOLD 0 // in MINUTES (Can't think of a pressure-related situation worth a notification)
#define HUMIDITY_ALARM_RANGE_LOWER 30.0F // s/b 30.0 - 50.0 per Mayo Clinic
#define HUMIDITY_ALARM_RANGE_UPPER 60.0F // 50.0 gave too many false alarms
#define HUMIDITY_ALARM_EMAIL_THRESHOLD 60 // in MINUTES

#endif // _CONFIG_H_