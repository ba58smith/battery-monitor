#ifndef _INTERNET_H_
#define _INTERNET_H_

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <InfluxDbClient.h>
#include <EMailSender.h>
#include "config.h"
#include "ui.h"
#include "packet_list.h"

/**
 * @brief Class that manages all connections to, and interactions with, the Internet.
 */

class Internet {

private:
    const char* wifi_ssid_ = SSID;
    const char* wifi_pw_ = PASSWORD;
    UI* ui_;
    InfluxDBClient* influxdb_;
    EMailSender* email_sender_;
    String email_recipient_ = String(EMAIL_RECIPIENT);
    EMailSender::EMailMessage email_message_;
    EMailSender::Response email_response_;

    /**
     * @brief The function that will ultimately be run as a Task,
     * every minute. (But only after being called in start_task_impl(), below.)
     */
    
    void handle_influx_queue_task() {
        while (1) {
            this->handle_influx_queue();
            vTaskDelay(60000 / portTICK_RATE_MS);
        }
    }

    /**
     * @brief Allows handle_influx_queue_task(), above, to be called from
     * within xTaskCreate from inside a class method.
     * https://stackoverflow.com/questions/45831114
     */
    
    static void start_handle_influx_queue_task(void* _this) {
        static_cast<Internet*>(_this)->handle_influx_queue_task();
    }

public:
    
    /**
     * @brief Construct a new Internet object.
     */
    Internet(UI* ui) : ui_{ui} {
        influxdb_ = new InfluxDBClient(INFLUXDB_URL, INFLUXDB_DB_NAME);
        influxdb_->setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);
        email_sender_ = new EMailSender(EMAIL_SENDER_ADDRESS, GMAIL_APP_PASSWORD);
        email_message_.subject = "Message from LoRa Receiver";
        email_message_.mime = MIME_TEXT_PLAIN;
    }

    /**
     * @brief Starts the task that sends new packets from the Influx queue to InfluxDB.
     * https://stackoverflow.com/questions/45831114
     */
    
    void start_tasks() {
        xTaskCreate(this->start_handle_influx_queue_task, "handle_influx_queue", 10000, this, 1, NULL);
    }

    /**
     * @brief Called during setup() and before sending data online, if necessary.
     */

    bool connect_to_wifi() {
        uint8_t attempts = 0;
        Serial.print("Connecting to wifi ");
        ui_->update_status_line("Connecting to wifi...");
        WiFi.begin(wifi_ssid_, wifi_pw_);
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            Serial.print(". ");
            delay(500);
            attempts++;
        }
        Serial.println("");
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Not connected to wifi");
            ui_->update_status_line("Connection failed.");
            delay(500);
            return false;
        }
        else {
            Serial.println("Connected to " + WiFi.localIP().toString());
            ui_->update_status_line("Connected to wifi.");
            configTime(-18000, 3600, "pool.ntp.org"); // Connect to NTP server with -5 TZ offset (-18000), 1 hr DST offset (3600).
            return true;
        }
    }

    /**
     * @brief Set as an xTask to run a few times per minute, to check for new packets in
     * the influx queue, and send them to InfluxDB.
     */

    void handle_influx_queue() {
        if (WiFi.status() == WL_CONNECTED) {
            Packet_t packet;
            while (read_packet_from_influx_queue(&packet)) {
                send_one_packet_to_influx(packet.data_source, packet.data_name, packet.data_value, packet.alarm_code,
                                          packet.RSSI, packet.SNR);
            }
        }
    }

    /**
     * @brief Sends one datapoint to InfluxDB 
     */

    bool send_one_packet_to_influx(String data_source, String data_name, String data_value, uint16_t alarm_code = 0,
                             int8_t RSSI = 0, int8_t SNR = 0) {
        Serial.println("Sending one new packet to InfluxDB");
        ui_->update_status_line("Sending to InfluxDB");
        Point packet("packets");
        packet.addTag("source", data_source);
        packet.addTag("name", data_name);
        packet.addField("value", data_value.toFloat());
        packet.addField("alarm", alarm_code);
        packet.addField("rssi", RSSI);
        packet.addField("snr", SNR);
        if (!influxdb_->writePoint(packet)) {
            Serial.println("InfluxDB write failed: " + influxdb_->getLastErrorMessage());
            ui_->update_status_line("InfluxDB write failed", 2);
            return false;
        }
        else {
            ui_->update_status_line("InfluxDB write successful");
            Serial.println("InfluxDB write successful");
            return true;
        }
    }

    String get_ssid() {
        return wifi_ssid_;
    }

    String get_ip() {
        return WiFi.localIP().toString();
    }

    bool connected_to_wifi() {
        return WiFi.status() == WL_CONNECTED;
    }

    /**
     * @brief Sends a text-only email. Used to notify user of an alarm
     * condition that has not cleared in a timely manner.
     * @param first_packet An iterator to PacketList::packets.begin()
     * @param end_of_packets An iterator to PacketList::packets.end()
     */

    void send_alarm_email(Packet_it_t first_packet, Packet_it_t end_of_packets) {
        Serial.println("Looking for alarms that need an email sent");
        ui_->update_status_line("Look for old alarms", 2);
        // create a time_t (which is the number of seconds since 1/1/1970) called "now"
        time_t now;
        // set "now" to the current time
        time(&now);
        bool email_attempted = false;
        for (Packet_it_t it = first_packet; it != end_of_packets; ++it) {
            uint64_t threshold = it->alarm_email_counter * it->alarm_email_threshold * 60;
            if (it->first_alarm_time > 0 && it->first_alarm_time + threshold < now) {
                tm* first_alarm = localtime(&it->first_alarm_time);
                String message_text = ui_->date_time_str() + " (Msg # " + it->alarm_email_counter + ")\n" 
                                    + it->data_source + " " + it->data_name + ": " + it->data_value 
                                    + "\nAlarm condition began on\n" + ui_->date_time_str(first_alarm);
                Serial.println(message_text);
                email_message_.message = message_text;
                email_response_ = email_sender_->send(email_recipient_, email_message_);
                email_attempted = true;
                Serial.println("Sending email");
                Serial.println("email_response_.status: " + email_response_.status);
                Serial.println("email_response_.code: " + email_response_.code);
                Serial.println("email_response_.desc: " + email_response_.desc);
                if (email_response_.code.toInt() == 0) { // email sent successfully
                    it->alarm_email_counter++;
                }
                ui_->sound_alarm(it->alarm_code);
            }
        }
        if (!email_attempted) {
            Serial.println("No emails needed");
        }
        ui_->update_status_line("Waiting for data");
        
    }

}; // class Internet

#endif // _INTERNET_H_