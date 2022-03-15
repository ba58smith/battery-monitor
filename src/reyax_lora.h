#ifndef _REYAX_LORA_H_
#define _REYAX_LORA_H

#include "Arduino.h"

class ReyaxLoRa
{
public:
    ReyaxLoRa(uint8_t pin, uint8_t network_id, uint8_t node_address, uint8_t base_station_address)
        : pin_{pin}, network_id_{network_id}, node_address_{node_address},
          base_station_address_{base_station_address}
    {}

    void initialize() {
        pinMode(pin_, OUTPUT);

        // Serial2 is defined in HardwareSerial.cpp as txPin = 17 and rxPin = 16
        Serial2.begin(115200);
        read_reply();
        delay(500);

        // Turn on the LoRa radio via transistor
        digitalWrite(pin_, HIGH);
        delay(500);

        // Wake up the LoRa and show the responses in the Serial Monitor
        send_and_read_reply("AT");
        send_and_read_reply("AT+VER?");
    }

    void one_time_setup() {
        String network_string = "AT+NETWORKID=" + String(network_id_);
        send_and_read_reply(network_string);
        String address_string = "AT+ADDRESS=" + String(node_address_);
        send_and_read_reply(address_string);
    }

    /**
     * @brief - read_reply() - if delay_ms is > 0, does a delay(delay_ms), then reads the reply from an
     * AT command that was just sent, then displays it on Serial. Some of the AT commands seem to need
     * the delay before reading the reply. (AT+SEND is one of them.)
     */

    void read_reply(int delay_ms = 0) {
        if (delay_ms)
            delay(delay_ms);
        Serial.println(Serial2.readStringUntil('\n'));
    }

    void send_and_read_reply(String send_string, int delay_ms = 0) {
        String command = send_string + String("\r\n");
        String sending = String("Sending: ") + command;
        Serial.println(sending);
        Serial2.print(command);
        read_reply(delay_ms);
    }

    void send_data(float voltage) {
        // 2 is Jim's network number, used by his code to "separate" my network from his.
        // The correct command below should be "AT+SEND=10,5,<voltage>", and the receiver code
        // needs to just drop the lines that deal with the so-called network ID.
        // String(voltage, 2); makes voltage always have two decimal places.
        String volt_str = String(voltage, 2);
        // BAS: remove the "+ 2" after fixing the base station code to not expect the "network"
        uint8_t data_length = volt_str.length() + 2;
        String payload = "AT+SEND=" + String(base_station_address_) + "," 
                         + String(data_length) + "," 
                         + String(network_id_) + "," 
                         + volt_str;
        send_and_read_reply(payload, 500);
    }

    void turn_off() {
        digitalWrite(pin_, LOW);
    }

private:
    uint8_t pin_;
    uint8_t network_id_;
    uint8_t node_address_;
    uint8_t base_station_address_;

}; // class ReyaxLoRa

#endif // _REYAX_LORA_H_
