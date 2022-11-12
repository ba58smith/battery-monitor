#ifndef _PACKET_LIST_H_
#define _PACKET_LIST_H_

#include <Arduino.h>
#include <list>
#include "packet_t.h"
#include "config.h"
#include "alarm.h"
#include "ui.h"
#include "queues.h"

#include <Adafruit_BME280.h>

/**
 * @brief PacketList is a class that manages all of the packets of data that are going to be
 * displayed on the OLED. Packets are received from the transmitters, and they come from any physical
 * sensors attached to the receiver PCB, and they can be used to show the status of things (like
 * info about the last web update). A packet contains all we want to know about a single datapoint, 
 * such as "Boat voltage" or "Pool water temperature". This class handles the
 * receipt of a new packet through Serial2 (via the LoRa radio) and the adding or updating of the new
 * packet in the std::list of packets. It also provides a way to add packets that don't come in from
 * the LoRa radio.
 */

class PacketList {

private:
    
    std::list<Packet_t> packets_;
    Packet_it_t loop_iterator_ = packets_.begin();
    UI* ui_;
    Adafruit_BME280* bme280_;

    /**
     * @brief The function that will ultimately be run as a Task,
     * every 500 ms. (But only after being called in start_task_impl(), below.)
     */
    
    void get_new_packets_task() {
        while (1) {
            this->get_new_packets();
            vTaskDelay(250 / portTICK_RATE_MS);
        }
    }

    void handle_packet_queue_task() {
        while (1) {
            this->handle_packet_queue();
            vTaskDelay(250 / portTICK_RATE_MS);
        }
    }

    /**
     * @brief Allows get_new_packets_task(), above, to be called from
     * within xTaskCreate from inside a class method.
     * https://stackoverflow.com/questions/45831114
     */
    
    static void start_get_new_packets_task_impl(void* _this) {
        static_cast<PacketList*>(_this)->get_new_packets_task();
    }

    /**
     * @brief Allows handle_packet_queue_task(), above, to be called from
     * within xTaskCreate from inside a class method.
     * https://stackoverflow.com/questions/45831114
     */
    
    static void start_handle_packet_queue_task(void* _this) {
        static_cast<PacketList*>(_this)->handle_packet_queue_task();
    }

public:
   /**
    * @brief Construct a new PacketList object.
    */

    PacketList(UI* ui, Adafruit_BME280* bme280) : ui_{ui}, bme280_{bme280} {}

    /**
     * @brief Starts the task that reads new packets coming in from the LoRa,
     * and the task that moves new packets from the new task queue into PacketList.
     * https://stackoverflow.com/questions/45831114
     */
    
    void start_tasks() {
        xTaskCreate(this->start_get_new_packets_task_impl, "get_new_packets", 10000, this, 2, NULL);
        xTaskCreate(this->start_handle_packet_queue_task, "handle_packet_queue", 10000, this, 1, NULL);
    }

    /**
     * @brief Setup this method in an xTask to run a few times per second, to check for new packets in
     * the new packet queue, and add them to (or update them in) PacketList.
     */

    void handle_packet_queue() {
       Packet_t packet;
       while (read_packet_from_queue(&packet)) {
           add_packet_to_list(&packet);
        }
    }

    /**
     * @brief Start the BME280. It doesn't happen if it's inside
     * the constructor, I think because that happens before setup()
     */

    void start_bme280() {
        bool success = bme280_->begin(0x76);
        if (!success) {
          Serial.println("Could not find a valid BME280 sensor, check wiring!");
          ui_->update_status_lines("BME280 error:", "check wiring");
        }
        else {
            Serial.println("BME280::begin() was successful");
        }
    }
   
    /**
    * @brief Populate a new Packet_t from the data coming in through Serial2, then add it
    * to, or update it in, the list of packets.
    */

    bool get_new_packets() {
       Packet_t new_packet;
       bool new_packet_received = false;
       String temp_str = "";
       
       while (Serial2.available()) {
           // If there are two packets in the Serial2 buffer, make sure the second one
           // starts at the right place. If it doesn't find a "+", it won't find a valid packet.
           String init_str = Serial2.readStringUntil('+');
           Serial.println("New data coming in");
           ui_->update_status_lines("New LoRa data", "coming in", 2);
           // see if the next 4 characters == "RCV="
           if (Serial2.readStringUntil('=') == "RCV") {
               Serial.println("It's a LoRa packet");
               initialize_packet(&new_packet);
               // make sure this is from one of OUR transmitters:
               new_packet.transmitter_address = Serial2.readStringUntil(',').toInt();
               Serial.println("Transmitter address = " + String(new_packet.transmitter_address));
               if (new_packet.transmitter_address >= ADDRESS_RANGE_LOWER 
                   && new_packet.transmitter_address <= ADDRESS_RANGE_UPPER) {
                   // now we know it's OK to process this packet
                   ui_->turnOnLed();
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
                   new_packet.data_source = Serial2.readStringUntil('%'); // "Bessie", "Pool", etc.
                   Serial.println("Transmitter name = " + new_packet.data_source);
                   if (new_packet.data_source.length() == 0) {
                       Serial.println("Error reading data_source from Serial2.");
                       return false;
                   }
                   new_packet.data_name = Serial2.readStringUntil('%'); // "Battery voltage", "Water temp", etc.
                   Serial.println("Data name = " + new_packet.data_name);
                   if (new_packet.data_name.length() == 0) {
                       Serial.println("Error reading data_name from Serial2.");
                       return false;
                   }
                   new_packet.data_value = Serial2.readStringUntil('%');
                   Serial.println("Data value = " + new_packet.data_value);
                   if (new_packet.data_value.length() == 0) {
                       Serial.println("Error reading data_value from Serial2.");
                       return false;
                   }
                   temp_str = Serial2.readStringUntil('%');
                   if (temp_str.length() == 0) {
                       Serial.println("Error reading alarm_code from Serial2.");
                       return false;
                   }
                   else {
                       Serial.println("Alarm code = " + temp_str);
                       new_packet.alarm_code = temp_str.toInt();
                       if (new_packet.alarm_code > 0) {
                           if (ui_->system_time_is_valid()) {
                               time(&new_packet.first_alarm_time); // set to current time
                               char *date = ctime(&new_packet.first_alarm_time);
                               Serial.println("First alarm time: " + String(date));
                           }
                           else {
                               new_packet.first_alarm_time = 0;
                               ui_->update_status_lines("System time invalid", "", 3);
                               Serial.println("System time invalid, first_alarm_time set to 0");
                           }
                       }
                   }
                   // this is the last bit of data in the <Data> portion, so we go back to "," as the separator
                   temp_str = Serial2.readStringUntil(',');
                   if (temp_str.length() == 0) {
                       Serial.println("Error reading alarm_email_threshold from Serial2.");
                       return false;
                   }
                   else {
                       Serial.println("Alarm email threshold = " + temp_str);
                       new_packet.alarm_email_threshold = temp_str.toInt();
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
                   
                   add_packet_to_queue(new_packet);
                   add_packet_to_influx_queue(new_packet);
                   new_packet_received = true;
                   ui_->update_status_lines("Waiting for data", "");
                   ui_->turnOFFLed();
               }
           }
       }
       if (new_packet_received) {
           return true;
       }
       else {
           return false;
       }
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
       packet->first_alarm_time = 0;
       packet->alarm_email_threshold = 0;
       packet->alarm_email_counter = 1; // 1, not 0
       packet->RSSI = 0;
       packet->SNR = 0;
       packet->timestamp = 0;
       packet->sent_to_influx = false;
    }
   
    /**
    * @brief Create a new "generic" packet with data from any source, then call add_packet_to_queue().
    * 
    * @param source - "Truck" or "Boat" or "Pool", etc.
    * @param name_of_data  "Voltage", "Water temp", etc.
    * @param value Obvious
    * @param alarm Alarm code
    * @param alarm_email_threshold # of minutes an alarm condition must exist before an email is sent
    */

    void create_generic_packet(String id, String source, String name_of_data, String value, int16_t alarm, uint16_t alarm_threshold = 0) {
       Packet_t new_packet;
       new_packet.unique_id = id;
       new_packet.data_source = source;
       new_packet.data_name = name_of_data;
       new_packet.data_value = value;
       new_packet.alarm_code = alarm;
       if (new_packet.alarm_code > 0) {
           if (ui_->system_time_is_valid()) {
              time(&new_packet.first_alarm_time); // set first alarm time to current time
           }
           else {
              new_packet.first_alarm_time = 0;
              ui_->update_status_lines("System time invalid", "", 3);
              Serial.println("System time invalid, first_alarm_time set to 0");
           }
       }
       new_packet.alarm_email_threshold = alarm_threshold;
       new_packet.timestamp = millis();
       add_packet_to_queue(new_packet);
       add_packet_to_influx_queue(new_packet);
    }
   
    /**
    * @brief Add a new packet to the list, or update the list if there is already a packet in it for
    * the same datapoint as the new packet.
    */

    void add_packet_to_list(Packet_t* packet) {
       if (packets_.empty()) {
           Serial.println("Adding first packet: " + packet->data_source + " " + packet->data_name);
           packets_.push_back(*packet); // add it to the list
           uint8_t list_size = packets_.size();
           Serial.println("New packet count: " + String(list_size));
       }
       else { // look for the packet in the list
           bool message_found = false;
           for (Packet_it_t it = packets_.begin(); it != packets_.end(); ++it) {
               if (it->unique_id == packet->unique_id) { // this packet is already in the list
                   // update the data that's different with each packet from the same datapoint
                   it->data_value = packet->data_value;
                   if (!packet->alarm_code) { // there is no alarm
                       it->first_alarm_time = 0;
                       it->alarm_email_counter = 1; // reset to new packet value: (1, not 0)
                   }
                   else if (!it->alarm_code && packet->alarm_code) { // alarm code is going from 0 to non-zero
                       it->alarm_has_sounded = false;
                       it->first_alarm_time = packet->first_alarm_time;
                   }
                   // edge case: datapoint has been in an alarm state, but the system time has been invalid,
                   // so first_alarm_time has not been set yet. See if the system time is now valid, and if
                   // it is, set first_alarm_time.
                   else if (it->alarm_code && packet->alarm_code && it->first_alarm_time == 0) {
                       if (ui_->system_time_is_valid()) {
                           time(&it->first_alarm_time); // set first alarm time to current time
                       }
                   }
                   it->alarm_code = packet->alarm_code;
                   it->RSSI = packet->RSSI;
                   it->SNR = packet->SNR;
                   it->timestamp = packet->timestamp;
                   it->sent_to_influx = false;
                   message_found = true;
                   Serial.println("Updating packet: " + packet->data_source + " " + packet->data_name);
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
       // print_packet_list_contents(); // needed only for troubleshooting
    }

    /**
     * @brief Print out the fields of every packet in PacketList.
     */

    void print_packet_list_contents() {
        String output = "";
        for (Packet_it_t it = packets_.begin(); it != packets_.end(); ++it) {
            output = "ID:" + String(it->unique_id) + ",Address:" + String(it->transmitter_address) + ",length:" + String(it->data_length) 
            + ",Source:" + it->data_source + ",Name:" + it->data_name + ",Value:" + it->data_value + ",";
            Serial.print(output);
            output = "AlmCode:" + String(it->alarm_code) + ",AlmSnd:" + String(it->alarm_has_sounded) + ",FstAlmTime:" 
            + String(it->first_alarm_time) + ",";
            Serial.println(output);
            output = "   AlmThsld:" + String(it->alarm_email_threshold) + ",EmlCntr:" + String(it->alarm_email_counter) 
            + ",RSSI:" + String(it->RSSI) + ",SNR:" + String(it->SNR)
            + ",Time:" + String(it->timestamp) + ",Snt2Inflx:" + String(it->sent_to_influx);
            Serial.println(output);
        }    
    }
   
    /**
    * @brief Add or update current BME280 data to packet_list_
    */

    void update_BME280_packets() {
       ui_->turnOnLed();
       Serial.println("Updating BME280 data");
       ui_->update_status_lines("Updating BME280", "data");
       int16_t alarm = 0;
       float data = (bme280_->readTemperature() * 1.8) + 32.0;
       data = data + TEMP_CALIBRATION; // Corrects for individual BME280 - see config.h
       Serial.println("temperature: " + String(data, 1));
       if (data <= TEMP_ALARM_RANGE_LOWER || data >= TEMP_ALARM_RANGE_UPPER) {
           alarm = 123; // 1 short, 2 long, 3 short
       }
       create_generic_packet("BME280_temp","BME280", "Temp (F)", String(data, 0), alarm, TEMP_ALARM_EMAIL_THRESHOLD);
       alarm = 0;
       
       data = (bme280_->readPressure() * 0.0002953); // convert from Pascals to inches of mercury
       Serial.println("pressure: " + String(data, 2));
       if (data <= PRESSURE_ALARM_RANGE_LOWER || data >= PRESSURE_ALARM_RANGE_UPPER) {
           alarm = 123; // 1 short, 2 long, 3 short
       }
       create_generic_packet("BME280_press", "BME280", "Pressure", String(data, 2), alarm, PRESSURE_ALARM_EMAIL_THRESHOLD);
       alarm = 0;

       data = (bme280_->readHumidity());
       Serial.println("humidity: " + String(data, 1));
       if (data <= HUMIDITY_ALARM_RANGE_LOWER || data >= HUMIDITY_ALARM_RANGE_UPPER) {
           alarm = 123; // 1 short, 2 long, 3 short
       }
       create_generic_packet("BME280_humid", "BME280", "Humidity", String(data, 0), alarm, HUMIDITY_ALARM_EMAIL_THRESHOLD);
       alarm = 0;
       
       ui_->turnOFFLed();
       ui_->update_status_lines("Waiting for data", "");
    }

    /**
    * @brief Iterates through packets_ one packet at a time. Called in main.cpp to display the
    * contents of each packet for a few seconds.
    */

    Packet_it_t advance_one_packet() {
       if (loop_iterator_ == packets_.end()) {
           loop_iterator_ = packets_.begin();
       }
       return loop_iterator_++;
    }

    /**
    * @brief Used in main.cpp to see if there are packets to display
    */

    uint8_t packet_list_not_empty() {
       return !packets_.empty();
    }

    /**
     * @brief create an iterator to the beginning of PacketList
     */
    
    Packet_it_t get_packets_begin() {
        return packets_.begin();
    }

    /**
     * @brief create an iterator to the end of PacketList
     */
    
    Packet_it_t get_packets_end() {
        return packets_.end();
    }

}; // class PacketList

#endif // _PACKET_LIST_H_
