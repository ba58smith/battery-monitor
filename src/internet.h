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
    String email_recipient_ = "3172130876@msg.fi.google.com";
    EMailSender::EMailMessage email_message_;
    EMailSender::Response email_response_;
    String jims_website_url_ = "http://www.totalcareprog.com/cgi-bin/butchmonitor_save.php";

public:
    
    /**
     * @brief Construct a new Internet object.
     */
    Internet(UI* ui) : ui_{ui} {
        influxdb_ = new InfluxDBClient(INFLUXDB_URL, INFLUXDB_DB_NAME);
        influxdb_->setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);
        email_sender_ = new EMailSender("ba58smith@gmail.com", "lsqxrdaljhluazgj");
        email_message_.subject = "Message from LoRa Receiver";
        email_message_.mime = MIME_TEXT_PLAIN;
    }

    /**
     * @brief Called during setup() and before sending data online, if necessary.
     */

    bool connect_to_wifi() {
        uint8_t attempts = 0;
        Serial.print("Connecting to wifi ");
        WiFi.begin(wifi_ssid_, wifi_pw_);
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            Serial.print(". ");
            delay(500);
            attempts++;
        }
        Serial.println("");
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Not connected to wifi");
            delay(500);
            return false;
        }
        else {
            Serial.println("Connected to " + WiFi.localIP().toString());
            return true;
        }
    }


    bool transmit_to_web() {
        Serial.println("Transmitting to Jim's website");
        bool http_success = false;
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("wifi not connected");
            if (!connect_to_wifi()) {
                return false;
            }
        }
        else {
            ui_->turnOnLed();
            ui_->update_status_line("Transmitting to web");
            Serial.println("Connected to wifi");
            HTTPClient http;
            String serverPath = jims_website_url_ + "?battery1=" + String(battery1, 2) 
                              + "&battery2=" + String(battery2, 2) + "&battery3=" + String(battery3) 
                              + "&temp=" + String(yourTemp) + "&humidity=" + String(yourHumidity) 
                              + "&pressure=" + String(yourPressure, 2);
            http.begin(serverPath.c_str());
            int httpResponseCode = http.GET();
            if (httpResponseCode > 0) {
                Serial.println("HTTP Response code: " + httpResponseCode);
                Serial.println(http.getString());
                http_success = true;
            }
            else {
                Serial.println("Error code: " + httpResponseCode);
            }
            // Free resources
            http.end();
        }
        ui_->update_status_line("Waiting for data");
        ui_->turnOFFLed(); // both LED's
        return http_success;

    } // end transmit to web

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
        if (data_source != "Web") {
            packet.addField("value", data_value.toFloat());
        }
        else {
            packet.addField("string_value", data_value);
        }
        packet.addField("alarm", alarm_code);
        packet.addField("rssi", RSSI);
        packet.addField("snr", SNR);
        if (!influxdb_->writePoint(packet)) {
            Serial.println("InfluxDB write failed: " + influxdb_->getLastErrorMessage());
            ui_->update_status_line("InfluxDB write failed", 2);
            return false;
        }
        else {
            ui_->update_status_line("Waiting for data");
            Serial.println("InfluxDB write successful");
            return true;
        }
    }

    /**
     * @brief sends all unsent packets to InfluxDB
     * @param first_packet An iterator to PacketList::packets.begin()
     * @param end_of_packets An iterator to PacketList::packets.end()
     * @return true if successful, false if unsucessful for any reason
     */

    bool send_packets_to_influx(Packet_it_t first_packet, Packet_it_t end_of_packets) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("wifi not connected");
            if (!connect_to_wifi()) {
                return false;
            }
        }
        for (Packet_it_t it = first_packet; it != end_of_packets; ++it) {
            if (!it->sent_to_influx) {
                if (send_one_packet_to_influx(it->data_source, it->data_name, it->data_value, it->alarm_code,
                                    it->RSSI, it->SNR)) {
                    Serial.println("Packet successfully sent to InfluxDB: " + it->data_source + " " + it->data_name);
                    it->sent_to_influx = true;
                }
                else {    
                    Serial.println("send_one_packet_to_influx() failed");
                    return false;
                }
            }
        }
        return true;
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
        ui_->update_status_line("Looking 4 old alarms", 2);
        // create a time_t (which is the number of seconds since 1/1/1970) and set it to the current time
        time_t now;
        time(&now);
        for (Packet_it_t it = first_packet; it != end_of_packets; ++it) {
            if (it->first_alarm_time > 0 && it->first_alarm_time + (it->alarm_email_threshold * 60) < now) {
                tm* first_alarm = localtime(&it->first_alarm_time);
                String message_text = ui_->get_current_time() + "\n" + it->data_source + " " + it->data_name + ": " + it->data_value 
                                    + "\nAlarm condition began on\n" + ui_->get_date_time_str(first_alarm);
                Serial.println(message_text);
                email_message_.message = message_text;
                email_response_ = email_sender_->send(email_recipient_, email_message_);
                Serial.println("Sending email");
                Serial.println("email_response_.status: " + email_response_.status);
                Serial.println("email_response_.code: " + email_response_.code);
                Serial.println("email_response_.desc: " + email_response_.desc);
                if (email_response_.code == 0) {
                    it->first_alarm_time = 0; // so we don't keep sending the email, unless it's still in an
                                              // alarm state for another alarm_email_threshold minutes.
                }
            }
        }
        ui_->update_status_line("Waiting for data");
        
    }

}; // class Internet

#endif // _INTERNET_H_