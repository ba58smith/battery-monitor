#ifndef _ALARM_H_
#define _ALARM_H_

#include <Arduino.h>
#include "config.h"

class Alarm {

public:
    
    /**
     * @brief Construct a new Alarm object.
     * 
     * @param pin - GPIO pin that's connected to the buzzer.
     */
    Alarm(uint8_t pin) : pin_{pin} {
        pinMode(pin_, OUTPUT);
        digitalWrite(pin_, LOW);
    }

    void parse_alarm_code(uint16_t alarm_code) {
        if (alarm_code > 99) {
            first_alarm_count = alarm_code / 100 % 10;
            second_alarm_count = alarm_code / 10 % 10;
            third_alarm_count = alarm_code % 10;
        }
        else if (alarm_code > 9) {
            first_alarm_count = alarm_code / 10 % 10;
            second_alarm_count = alarm_code % 10;
        }
        else {
            first_alarm_count = alarm_code % 10;
        }
    }

    /**
    * @brief sound_alarm() turns the buzzer on and off for certain intervals
    * 
    * @param alarm_code is an int of 1, 2, or 3 digits:
    *    examples: 
    *    X sounds a short alarm (100 ms) X times
    *    XY sounds a short alarm the X times, and a long alarm (300 ms) Y times
    *    XYZ sounds a short alarm X times, a long alarm Y times, and a short alarm Z times 
    */

    void sound_alarm(uint16_t alarm_code) {
        if (alarm_code == 0) {
            return;
        }
        parse_alarm_code(alarm_code);
        // now that we have the alarm counts, sound the alarm(s)
        // first, the short beeps
        soundAlarm(20, first_alarm_count);
        // then the long beeps, if there are any
        if (second_alarm_count) {
            soundAlarm(150, second_alarm_count);
        }
        // finally, the last set of short beeps, if there are any
        if (third_alarm_count) {
            soundAlarm(20, third_alarm_count);
        }
    }

    /**
    * @brief Legacy alarm function, modified to work if called the old way,
    *        with no parameters.
    */

    void soundAlarm(uint16_t alarm_length = 100, uint16_t iterations = 2) {
        for (int i = 0; i < iterations; ++i) {
            digitalWrite(pin_, HIGH);
            delay(alarm_length);
            digitalWrite(pin_, LOW);
            delay(400);
        }
        digitalWrite(pin_, LOW);
    }

private:
    uint8_t pin_ = 0;
    uint8_t first_alarm_count = 0;
    uint8_t second_alarm_count = 0;
    uint8_t third_alarm_count = 0;

}; // class Alarm

#endif // _ALARM_H_