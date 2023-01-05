#ifndef _CONFIG_H_
#define _CONFIG_H_

// Copy default_config.h to a new file, secret_config.h. Then
// edit secret_config.h to include all of your sensitive info.
// secret_config.h will not be uploaded to GitHub.
#include "secret_config.h"

// Un-comment and change the baud rate below if you want to change it.
// Then upload and run, which will write the new baud rate to EEPROM.
// Then comment it out again, upload, and run - so it won't keep writing
// it to EEPROM every time it boots.
// #define LORA_BAUD_RATE 115200ULL     // default 115200

#define BASE_STATION

#define TEMP_CALIBRATION -1.0 // my particular BME280 reads 1.0 Fahrenheit too warm
// Home alarm ranges
#define LOW_TEMP_ALARM_VALUE 73.0F // s/b 73.0
#define HIGH_TEMP_ALARM_VALUE 88.0F // s/b 88.0
#define TEMP_ALARM_CODE 1
#define TEMP_ALARM_EMAIL_INTERVAL 120 // in MINUTES

#define LOW_PRESSURE_ALARM_VALUE 29.0F // inches of hg - normal is 29.0 - 31.0
#define HIGH_PRESSURE_ALARM_VALUE 31.0F // s/b 31.0
#define PRESSURE_ALARM_CODE 1
#define PRESSURE_ALARM_EMAIL_INTERVAL 0 // in MINUTES (0 means no email will be sent)

#define LOW_HUMIDITY_ALARM_VALUE 30.0F // s/b 30.0 - 50.0 per Mayo Clinic
#define HIGH_HUMIDITY_ALARM_VALUE 70.0F // s/b 60.0 (50.0 gave too many false alarms)
#define HUMIDITY_ALARM_CODE 1
#define HUMIDITY_ALARM_EMAIL_INTERVAL 120 // in MINUTES

#endif // _CONFIG_H_