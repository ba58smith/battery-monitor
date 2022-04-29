#ifndef _PACKET_T_H_
#define _PACKET_T_H_

#include <Arduino.h>
#include <list>

struct Packet_t {
        String unique_id = "";
        uint16_t transmitter_address = 0;
        int8_t data_length = 0;
        String data_source = "";
        String data_name = "";
        String data_value = "";
        int16_t alarm_code = 0;
        bool alarm_has_sounded = false;
        int8_t RSSI = 0;
        int8_t SNR = 0;
        uint32_t timestamp = 0;
};

typedef std::list<Packet_t>::iterator Packet_it_t;

#endif // #ifndef _PACKET_T_H_