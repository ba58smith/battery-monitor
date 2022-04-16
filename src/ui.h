#ifndef _UI_H_
#define _UI_H_

//#include "Arduino.h"
#include "config.h"
#include <Adafruit_SSD1306.h>
#include "alarm.h"
#include "packet_list.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 5

// a quick way to adjust the vertical seperation between the 
// text lines on the oled. So, below, line1 will start on y17, line2 on y32, etc.
#define line1 17
#define line2 32  // 20
#define line3 47  // 40
//#define line4 55

#define blueLED 26

void turnOnLed() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(blueLED, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on
    digitalWrite(blueLED, HIGH);
}

void turnOFFLed() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(blueLED, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW); // turn the LED on
    digitalWrite(blueLED, LOW);
}

/**
 * @brief UI is the class that controls the display and the alarm.
 *        It displays "status info" on the top (yellow) line, and cycles
 *        through all of the other data the receiver is getting from the
 *        transmitters. This class also controls the Alarm, which is used
 *        when a transmitter sends in data that is "out of range", but could
 *        also be used to alert of something like no wifi, or a failed web update.
 */

class UI {

private:
    Adafruit_SSD1306* display_ = NULL;
    Alarm* alarm_;

public:
    
    UI() {
        display_ = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
        alarm_ = new Alarm(buzzerPin);
    }

    void prepare_display() {
        if (display_->begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
            Serial.println("OLED successfully started");
        }
        else {
            Serial.println("SSD1306 allocation failed");
        }
        display_->clearDisplay();
        display_->setCursor(0,0);
        delay(2000); //BAS: is a delay necessary at all?
        // Draw a single pixel in white (dont recall why i need this, but when i took it out it broke)
        // BAS: surely this can't be required?
        display_->setTextSize(1);
        display_->drawPixel(0, 0, SSD1306_WHITE);
        display_->setTextColor(SSD1306_WHITE);
    }

    /**
     * @brief Updates the bottom 3 lines of the display to show everything about a single
     * datapoint.
     */
     
   void display_one_packet(Packet_t packet) {
       clear_packet_area();
       display_->setTextSize(1);
       display_->setCursor(0, line1);
       display_->print(packet.data_source);
       display_->print("-");
       display_->println(packet.data_name);
       display_->setCursor(0, line2);
       display_->print(packet.data_value);
       display_->print(" Age ");
       // BAS: make this display more than just seconds - see TWatchSK's "uptime"
       display_->print((millis() - packet.timestamp) / 1000);
       if (packet.alarm_code) {
           display_->print(" Alarm ");
           display_->print(packet.alarm_code);
       }
       display_->setCursor(0, line3);
       if (packet.RSSI != 0) { // this is a packet from a transmitter
           display_->print("RSSI/SNR: ");
           display_->print(packet.RSSI);
           display_->print("/");
           display_->print(packet.SNR);
       }
       display_->display();
       if (packet.alarm_code && !packet.alarm_has_sounded) {
           alarm_->sound_alarm(packet.alarm_code);
           packet.alarm_has_sounded = true;
       }
   }

   
   /**
    * @brief Updates the yellow status line (the top line of the OLED)
    * 
    * @param status_str - The string you want to display there - max is 21, I think (BAS?)
    * @param duration_seconds - # of seconds to display this string before the next update 
    * of the status line. If not specified, it's 1 second. If you want this string to stay
    * on the screen until the next explicit update, use 0.
    */

   void update_status_line(String status_str, uint16_t duration_seconds = 1) {
       clear_status_line();
       display_->print(status_str);
       display_->display();
       if (duration_seconds) {
           delay(duration_seconds * 1000);
       }
   }

    /**
     * @brief Display the status just before connecting to wifi.
     */
    void before_connect_to_wifi_screen() {
        update_status_line("Connecting to:");
        update_status_line(SSID);
    }

    /**
     * @brief Display the status just after connecting to wifi.
     */
    void after_connect_to_wifi_screen(String local_ip) {
        update_status_line("Connected to:");
        update_status_line(local_ip, 3);
        update_status_line("Waiting for data", 0);
    }

    
    /**
     * @brief Clears the top yellow line of the OLED and prepares it for the next text
     * to display there.
     */
    
    void clear_status_line() {
        display_->setCursor(0, 0);
        // This is how Jim did it:
        for (int y = 0; y <= 15; y++) {
            for (int x = 0; x < 127; x++) {
                display_->drawPixel(x, y, BLACK);
            }
        }
        // BAS: drawFastHLine causes a crash   
        //display_->drawFastHLine(0, 15, 15, SSD1306_BLACK);
        display_->setCursor(0, 0);
        display_->display();
    }

    void clear_packet_area() {
        for (int y = line1; y <= SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                display_->drawPixel(x, y, BLACK);
            }
        }
        // BAS: this crashes the system
        // display_->drawFastHLine(line1, 63, 48, SSD1306_BLACK);
        display_->display();
    }
    
}; // class UI

#endif // _UI_H_
