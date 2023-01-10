#ifndef _REYAX_LORA_H_
#define _REYAX_LORA_H_

#include "Arduino.h"
#include "config.h"

class ReyaxLoRa {
public:
    // Constructor for the transmitter. pin is the "power pin" for the LoRa radio.
    ReyaxLoRa(uint8_t pin)
        : pin_{pin}
    {}

    // Constructor for the receiver (no pin to turn it on/off - it's always powered on)
    ReyaxLoRa()
    {}

    /**
     * @brief - initialize() sends power to the LoRa radio if pin_ has been set
     * to something other than 0 in the constructor (which should be done ONLY
     * for a transmitter - the receiver's radio is always powered on), then it
     * starts Serial2, then it wakes up the radio.
     */

    void initialize() {
        Serial.println("lora::initialize()");
        if (pin_) {
            pinMode(pin_, OUTPUT);
            // Turn on the LoRa radio via transistor
            digitalWrite(pin_, HIGH);
            delay(200);
        }

        // Serial2 is defined in HardwareSerial.cpp as txPin = 17 and rxPin = 16
        Serial2.begin(115200);
        read_reply(); // the reply comes from Serial2.begin()
        delay(500);

        // Wake up the LoRa and show the responses in the Serial Monitor
        send_and_read_reply("AT");
        send_and_read_reply("AT+VER?");
        send_and_read_reply("AT+PARAMETER=9,7,1,4"); // SF, BW, CR, Preamble
        send_and_read_reply("AT+NETWORKID?");
        send_and_read_reply("AT+ADDRESS?");
        send_and_read_reply("AT+PARAMETER?");
    }

    
    /**
     * @brief - one_time_setup() writes the network ID, this ESP32's address,
     * and the baud rate (if not the default) to EEPROM. It should be called
     * only once for a new LoRa radio, or if any of these three parameters
     * is changed. Controlled by the #define LORA_SETUP_REQUIRED line near
     * the top of main.cpp.
     */

    void one_time_setup() {
        Serial.println("lora::one_time_setup()");
        uint8_t id = LORA_NETWORK_ID;
        String network_string = "AT+NETWORKID=" + String(id);
        send_and_read_reply(network_string);

        uint16_t address = LORA_NODE_ADDRESS;
        String address_string = "AT+ADDRESS=" + String(address);
        send_and_read_reply(address_string);
    #ifdef LORA_BAUD_RATE
        uint32_t baud = LORA_BAUD_RATE;
        String baud_rate_string = "AT+IPR=" + String(baud);
        send_and_read_reply(baud_rate_string);
    #endif        
    }

    /**
     * @brief set_frequency() is used only to change the default
     * frequency of 915000000 
     */

    void set_frequency(int64_t freq) {
        frequency_ = freq;
    }

    /**
     * @brief set_output_power() is used only to change the default
     * of 15
     */
    
    void set_output_power(int8_t power) {
        output_power_ = power;
    }

    /**
     * @brief set_spread_factor() is used only to change the default
     * of 12. The larger the SF is, the better the sensitivity is. But the transmission
     * time will take longer.
     * Possible values are 7 - 12.
     */
    
    void set_spread_factor(int8_t spread) {
        spread_factor_ = spread;
    }

    /**
     * @brief set_bandwidth() is used only to change the default
     * of 7.
     * <Bandwidth>0~9 list as below
     * 0 : 7.8KHz (not recommended, over spec.)
     * 1 : 10.4KHz (not recommended, over spec.)
     * 2 : 15.6KHz
     * 3 : 20.8 KHz
     * 4 : 31.25 KHz
     * 5 : 41.7 KHz
     * 6 : 62.5 KHz
     * 7 : 125 KHz
     * 8 : 250 KHz
     * 9 : 500 KHz
     */

    void set_bandwidth(int8_t bandwidth) {
        bandwidth_ = bandwidth;
    }

    /**
     * @brief set_coding_rate() is used only to change the default
     * of 1. A higher coding rate will not increase range, but will make a link more reliable,
     * and will slow down the transmission.
     * Values are 1 - 4. (Representing 4/5, 4/6, 4/7, and 4/8, I THINK)
     */

    void set_coding_rate(int8_t rate) {
        coding_rate_ = rate;
    }
    
    /**
     * @brief set_preamble() is used only to change the default
     * of 4
     */
    
    void set_preamble(int8_t preamble) {
        preamble_ = preamble;
    }

    /**
     * @brief - If delay_ms is > 0, does a delay(delay_ms), then reads whatever is in the
     * Serial2 buffer (typically, it's a reply from an AT command that was just sent), then displays
     * it on Serial. Some of the AT commands seem to need the delay before reading the reply.
     * (AT+SEND is one of them.)
     */

    void read_reply(int delay_ms = 0) {
        if (delay_ms)
            delay(delay_ms);
        Serial.println(Serial2.readStringUntil('\n'));
    }

    
    /**
     * @brief Sends an AT command to the LoRa, then reads the reply from the LoRa
     * and displays it in the serial monitor.
     * 
     * @param send_string The AT command string you want to send to the LoRa
     * @param delay_ms Delay between sending the AT command and trying to read the reply.
     * See read_reply(), above.
     */

    void send_and_read_reply(String send_string, int delay_ms = 0) {
        String command = send_string + "\r\n";
        String sending = "Sending: " + command;
        // Display it on the serial monitor
        Serial.println(sending);
        // Now send it to the LoRa
        Serial2.print(command);
        read_reply(delay_ms);
    }

    #ifndef BASE_STATION
     /**
     * @brief Sends voltage data from a transmitter to the base station
     * 
     * @param voltage 
     */

    void send_voltage_data(float voltage) {
        // String(voltage, 2); makes voltage always have two decimal places.
        String volt_str = String(voltage, 2);
        uint16_t alarm_code = 0;
        if (voltage < VOLTAGE_ALARM_RANGE_LOWER || voltage > VOLTAGE_ALARM_RANGE_UPPER) {
            alarm_code = VOLTAGE_ALARM_CODE;
        }
        uint16_t address = LORA_BASE_STATION_ADDRESS;
        String data_str = String(TRANSMITTER_NAME + "%Voltage%" + volt_str + "%" + alarm_code);
        uint8_t data_length = data_str.length();
        String payload = "AT+SEND=" + String(address) + ","
                         + String(data_length) + "," 
                         + data_str;
        send_and_read_reply(payload, 500);
    }
    #endif

    void turn_off() {
        digitalWrite(pin_, LOW);
    }

private:
    uint8_t pin_ = 0;
    // All variables below are set to the factory defaults.
    // Change any of them with the appropriate "set" method.
    int64_t frequency_ = 915000000;
    int32_t baud_rate_ = 115200;
    int8_t output_power_ = 15;
    int8_t spread_factor_ = 12;
    int8_t bandwidth_ = 7;
    int8_t coding_rate_ = 1;
    int8_t preamble_ = 4;

}; // class ReyaxLoRa

#endif // _REYAX_LORA_H_
