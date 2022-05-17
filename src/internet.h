#ifndef _INTERNET_H_
#define _INTERNET_H_

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <InfluxDbClient.h>
#include "config.h"
#include "ui.h"
#include "packet_list.h"

/**
 * @brief Class that manages all connections to, and interactions with, the Internet.
 */

class Internet {

private:
    const char* wifi_ssid_ = SSID; //"KeyAlmostWest";
    const char* wifi_pw_ = PASSWORD; //"sfaesfae";
    UI* ui_;
    InfluxDBClient* influxdb_;
    String jims_website_url_ = "http://www.totalcareprog.com/cgi-bin/butchmonitor_save.php";

public:
    
    /**
     * @brief Construct a new Internet object.
     */
    Internet(UI* ui) : ui_{ui} {
        influxdb_ = new InfluxDBClient(INFLUXDB_URL, INFLUXDB_DB_NAME);
        influxdb_->setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);
    }

    /**
     * @brief Called during setup() and by transmitToWeb if necessary.
     */

    void connect_to_wifi() {
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
        }
        else {
            Serial.println("Connected to " + WiFi.localIP().toString());
        }
    }

    // BAS: if I start sending data to Arduino IO, this will go away.
    // But keep it during the transition.
    bool transmit_to_web() {
        Serial.println("Transmitting to Jim's website");
        bool success = false;
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("wifi not connected");
            connect_to_wifi();
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
            // Your Domain name with URL path or IP address with path
            http.begin(serverPath.c_str());
            // Send HTTP GET request //BAS: save httpResponseCode as a data member - Internet::last_reponse_code_ and add to packet&display
            int httpResponseCode = http.GET();
            if (httpResponseCode > 0) {
                Serial.print("HTTP Response code: ");
                Serial.println(httpResponseCode);
                Serial.println(http.getString());
                success = true;
            }
            else {
                Serial.print("Error code: ");
                Serial.println(httpResponseCode);
            }
            // Free resources
            http.end();
        }
        ui_->update_status_line("Waiting for data");
        ui_->turnOFFLed(); // both LED's
        return success;

    } // end transmit to web

    /**
     * @brief Sends one datapoint to InfluxDB 
     */

    bool one_packet_to_influx(String data_source, String data_name, String data_value, uint16_t alarm_code = 0,
                             int8_t RSSI = 0, int8_t SNR = 0) {
        Serial.println("Sending one new packet to InfluxDB");
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
            Serial.print("InfluxDB write failed: ");
            Serial.println(influxdb_->getLastErrorMessage());
            return false;
        }
        else {
            return true;
        }
    }

    /**
     * @brief sends all unsent packets to InfluxDB 
     */

    void send_packets_to_influx(Packet_it_t first_packet, Packet_it_t end_of_packets) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("wifi not connected");
            connect_to_wifi();
        }
        for (Packet_it_t it = first_packet; it != end_of_packets; ++it) {
            if (!it->sent_to_influx) {
                bool successful = one_packet_to_influx(it->data_source, it->data_name, it->data_value, it->alarm_code,
                                    it->RSSI, it->SNR);
                if (successful) {
                    it->sent_to_influx = true;
                }
                else {    
                    Serial.println("one_packet_to_influx() failed");
                    break;
                }
            }
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

}; // class Internet

#endif // _INTERNET_H_