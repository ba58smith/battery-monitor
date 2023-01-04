#ifndef _CONFIG_H_
#define _CONFIG_H_

// Comment out the "secret_config.h" line, and UN-comment
// the "default_config.h" line. Then edit the default_config.h
// file to include all of your sensitive information.
#include "secret_config.h"
// #include "default_config.h"

#define BASE_STATION

// Un-comment and change the baud rate below if you want to change it.
// Then upload and run, which will write the new baud rate to EEPROM.
// Then comment it out again, upload, and run - so it won't keep writing
// it to EEPROM every time it boots.
// #define LORA_BAUD_RATE 115200ULL     // default 115200


// Home alarm ranges
#define TEMP_CALIBRATION -1.0 // my particular BME280 reads 1.0 Fahrenheit too warm
#define TEMP_ALARM_RANGE_LOWER 73.0F // s/b 73.0
#define TEMP_ALARM_RANGE_UPPER 88.0F // s/b 88.0
#define TEMP_ALARM_EMAIL_THRESHOLD 120 // in MINUTES
#define PRESSURE_ALARM_RANGE_LOWER 29.0F // inches of hg - normal is 29.0 - 31.0
#define PRESSURE_ALARM_RANGE_UPPER 29.0F // s/b 31.0 // BAS: change back to 31.0 after testing
#define PRESSURE_ALARM_EMAIL_THRESHOLD 240 // in MINUTES BAS: change back to 0 after testing
#define HUMIDITY_ALARM_RANGE_LOWER 30.0F // s/b 30.0 - 50.0 per Mayo Clinic
#define HUMIDITY_ALARM_RANGE_UPPER 70.0F // s/b 60.0 (50.0 gave too many false alarms)
#define HUMIDITY_ALARM_EMAIL_THRESHOLD 120 // in MINUTES

#endif // _CONFIG_H_