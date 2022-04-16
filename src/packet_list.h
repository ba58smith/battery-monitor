#ifndef _PACKET_LIST_H_
#define _PACKET_LIST_H_

#include <Arduino.h>
#include <list>
#include "config.h"
#include "alarm.h"
#include "functions.h"
#include <Adafruit_BME280.h>

uint16_t yourTemp = 80;
uint16_t yourPressure = 1022;
uint16_t yourHumidity = 50;

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

/**
 * @brief PacketList is a class that manages all of the packets of data that are going to be
 * displayed on the OLED. Packets are received from the transmitters, and they come from any physical
 * sensors attached to the receiver PCB, and they can be used to show the status of things (like
 * info about the last web update). A packet contains all we want to know about a single datapoint, 
 * such as "Boat voltage" or "Pool water temperature" or "Last Web Update". This class handles the
 * receipt of a new packet through Serial2 (via the LoRa radio) and the adding or updating of the new
 * packet in the std::list of packets.
 */

class PacketList {

private:
    
    std::list<Packet_t> packets_;
    std::list<Packet_t>::iterator loop_iterator_ = packets_.begin();
    Adafruit_BME280* bme280_;

public:
   /**
    * @brief Construct a new PacketList object.
    */

    PacketList() {
        bme280_ = new Adafruit_BME280();
        if (!bme280_->begin(0x76)) {
          Serial.println("Could not find a valid BME280 sensor, check wiring!");
          // BAS: display something on the top line about the error.
        }
    }

   
   /**
    * @brief Populate a new Packet_t from the data coming in through Serial2.
    */

   bool get_new_packets() {
       Packet_t new_packet;
       bool new_packet_received = false;
       String temp_str = "";
       
       while (Serial2.available()) {
           // BAS: Add code here to display "Receiving data" on the top line of display
           // If there are two packets in the Serial2 buffer, make sure the second one
           // starts at the right place. If it doesn't find a "+", it won't find a valid packet.
           String init_str = Serial2.readStringUntil('+');
           Serial.println("New packet coming in");
           // see if the next 4 characters == "RCV="
           if (Serial2.readStringUntil('=') == "RCV") {
               Serial.println("New packet received");
               initialize_packet(&new_packet);
               // make sure this is from one of OUR transmitters:
               new_packet.transmitter_address = Serial2.readStringUntil(',').toInt();
               Serial.println("Transmitter address = " + new_packet.transmitter_address);
               if (new_packet.transmitter_address >= ADDRESS_RANGE_LOWER 
                   && new_packet.transmitter_address <= ADDRESS_RANGE_UPPER) {
                   // now we know it's OK to process this packet
                   //turnOnLed();
                   temp_str = Serial2.readStringUntil(',');
                   if (temp_str.length() == 0) {
                       Serial.println("Error reading data_length from Serial2.");
                       return false;
                   }
                   else {
                       Serial.println("Data length = " + temp_str);
                       new_packet.data_length = temp_str.toInt();
                   }
                   // Now we're into the <Data> portion of the packet, which is separated into smaller bits of
                   // data by the % separator.
                   new_packet.data_source = Serial2.readStringUntil('%'); // "Bessie" or "Pool"
                   Serial.println("Transmitter name = " + new_packet.data_source);
                   if (new_packet.data_source.length() == 0) {
                       Serial.println("Error reading data_source from Serial2.");
                       return false;
                   }
                   new_packet.data_name = Serial2.readStringUntil('%'); // "Battry voltage" or "Water temp"
                   Serial.println("Data name = " + new_packet.data_name);
                   if (new_packet.data_name.length() == 0) {
                       Serial.println("Error reading data_name from Serial2.");
                       return false;
                   }
                   new_packet.data_value = Serial2.readStringUntil('%'); // Keep it as a String for now, even though it's
                                                              // probably a number. Convert it to a number before
                                                              // sending to Adafruit IO if necessary.
                   Serial.println("Data value = " + new_packet.data_value);
                   if (new_packet.data_value.length() == 0) {
                       Serial.println("Error reading data_value from Serial2.");
                       return false;
                   }
                   // this is the last bit of data in the <Data> portion, so we go back to "," as the separator
                   temp_str = Serial2.readStringUntil(',');
                   if (temp_str.length() == 0) {
                       Serial.println("Error reading alarm_code from Serial2.");
                       return false;
                   }
                   else {
                       Serial.println("Alarm code = " + temp_str);
                       new_packet.alarm_code = temp_str.toInt();
                   }
                   temp_str = Serial2.readStringUntil(',');
                   if (temp_str.length() == 0) {
                       Serial.println("Error reading RSSI from Serial2.");
                       return false;
                   }
                   else {
                       Serial.println("RSSI = " + temp_str);
                       new_packet.RSSI = temp_str.toInt();
                   }
                   // last bit of data in the Packet, so read until '\n'
                   temp_str = Serial2.readStringUntil('\n');
                   if (temp_str.length() == 0) {
                       Serial.println("Error reading SNR from Serial2.");
                       return false;
                   }
                   else {
                       Serial.println("SNR = " + temp_str);
                       new_packet.SNR = temp_str.toInt();
                   }
                   new_packet.unique_id = String(new_packet.transmitter_address) + new_packet.data_name;
                   new_packet.timestamp = millis();
                   
                   add_packet_to_list(&new_packet);
                   new_packet_received = true;
                                      
                   delay(1000); // to keep the LED on for more than a few ms
                   //turnOFFLed();
               }
           }
       }
       return new_packet_received;
   }

   
   /**
    * @brief Set all data members to blank or 0.
    */

   void initialize_packet(Packet_t* packet) {
       packet->unique_id = "";
       packet->transmitter_address = 0;
       packet->data_length = 0;
       packet->data_source = "";
       packet->data_name = "";
       packet->data_value = "";
       packet->alarm_code = 0;
       packet->alarm_has_sounded = false;
       packet->RSSI = 0;
       packet->SNR = 0;
       packet->timestamp = 0;
   }

   
   /**
    * @brief Create a new packet with data from any source, then call add_packet_to_list().
    * 
    * @param source - "Truck" or "Boat" or "Pool", etc.
    * @param name_of_data  "Voltage", "Water temp", etc.
    * @param value Obvious
    * @param alarm Alarm code
    */

   void create_new_packet(String source, String name_of_data, String value, int16_t alarm) {
       Packet_t new_packet;
       new_packet.unique_id = source + name_of_data;
       new_packet.data_source = source;
       new_packet.data_name = name_of_data;
       new_packet.data_value = value;
       new_packet.alarm_code = alarm;
       new_packet.timestamp = millis();
       add_packet_to_list(&new_packet);
   }

   
   /**
    * @brief Add a new packet to the list, or update the list if there is already a packet in it for
    * the same datapoint as the new packet.
    */

   void add_packet_to_list(Packet_t* packet) {
       if (packets_.empty()) {
           Serial.println("Adding packet: " + packet->data_source + " " + packet->data_name);
           packets_.push_back(*packet); // add it to the list
           uint8_t list_size = packets_.size();
           Serial.println("New packet count: " + String(list_size));
       }
       else {
           bool message_found = false;
           for (std::list<Packet_t>::iterator it = packets_.begin(); it != packets_.end(); ++it) {
               if (it->unique_id == packet->unique_id) { // this packet is already in the list
                   // update the data that's different with each packet from the same datapoint
                   it->data_value = packet->data_value;
                   it->alarm_code = packet->alarm_code;
                   if (packet->alarm_code == 0) {
                       it->alarm_has_sounded = false;
                   }
                   it->RSSI = packet->RSSI;
                   it->SNR = packet->SNR;
                   it->timestamp = packet->timestamp;
                   message_found = true;
                   break;
               }
           }
           if (!message_found) { // it's not already in the list
               Serial.println("Adding packet: " + packet->data_source + " " + packet->data_name);
               packets_.push_back(*packet); // add it to the list
               uint8_t list_size = packets_.size();
               Serial.println("New packet count: " + String(list_size));
           }
       }
   }

   
   /**
    * @brief Update the alarm_has_sounded member of Packet_t, so that it can be done
    * from the UI.
    * BAS: is this necessary? Can I modify the packet that's sent to print, back in packets?
    */

   void set_alarm_sounded(String id) {
       //Start an iterator, find the unique_id, update the field
   }

   
   /**
    * @brief Add or update current BME280 data to packet_list_
    */

   void update_BME280_packets() {
       Serial.println("Updating BME280 data");
       int16_t alarm = 0;
       float data = (bme280_->readTemperature() * 1.8) + 32;
       Serial.println("temperature: " + String(data));
       if (data <= TEMP_ALARM_RANGE_LOWER || data >= TEMP_ALARM_RANGE_UPPER) {
           alarm = 123; // 1 short, 2 long, 3 short
       }
       // BAS: get rid of next line when you stop updating Jim's website
       yourTemp = data;
       create_new_packet("BME280", "Temp (F)", (String)data, alarm);
       
       data = (bme280_->readPressure() / 100.0);
       Serial.println("pressure: " + String(data));
       if (data <= PRESSURE_ALARM_RANGE_LOWER || data >= PRESSURE_ALARM_RANGE_UPPER) {
           alarm = 123; // 1 short, 2 long, 3 short
       }
       // BAS: get rid of next line when you stop updating Jim's website
       yourPressure = data;
       create_new_packet("BME280", "Pressure (hPa)", (String)data, alarm);

       data = (bme280_->readHumidity());
       Serial.println("humidity: " + String(data));
       if (data <= HUMIDITY_ALARM_RANGE_LOWER || data >= HUMIDITY_ALARM_RANGE_UPPER) {
           alarm = 123; // 1 short, 2 long, 3 short
       }
       // BAS: get rid of next line when you stop updating Jim's website
       yourHumidity = data;
       create_new_packet("BME280", "Humidity", (String)data, alarm);
   }

   /**
    * @brief Create or update the packet that tells when the last web successful web update happened
    * 
    * @param last_successful_update - updated every time the web update is successful
    */

   void update_web_update_packet(uint64_t last_successful_update) { //String source, String name_of_data, String value, int16_t alarm
       int16_t alarm = 0;
       String source = "Web";
       String name_of_data = "Last update";
       String value = "12/31 @ 23:59"; // BAS: try to display actual date and time (TWatchSK gui.cpp line 801)
       if (millis() - last_successful_update > WEB_UPDATE_ALARM_AGE) {
           alarm = 333;
       }
       create_new_packet(source, name_of_data, value, alarm);
   }

   /**
    * @brief Iterates through packets_ one packet at a time. Called in main.cpp to display the
    * contents of each packet for a few seconds.
    */

   Packet_t advance_one_packet() {
       if (loop_iterator_ == packets_.end()) {
           loop_iterator_ = packets_.begin();
       }
       return *loop_iterator_++;
   }

   
   /**
    * @brief Used in main.cpp to see if there are packets to display
    */
   uint8_t get_packet_list_size() {
       return packets_.size();
   }

   /**
    * @brief Make some fake packets for testing
    */

   void make_fake_packets() { //String source, String name_of_data, String value, int16_t alarm
       create_new_packet("Truck", "Voltage", "12.60", 0);
       create_new_packet("Killer", "Voltage", "12.00", 0);
   }

}; // class PacketList

#endif // _PACKET_LIST_H_
