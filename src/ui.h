#ifndef _UI_H_
#define _UI_H_

//#include "Arduino.h"
#include "config.h"
#include "packet_t.h"
#include "packet_list.h"
#include <Adafruit_SSD1306.h>
#include "alarm.h"
#include "internet.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 5

// a quick way to adjust the vertical seperation between the 
// text lines on the oled. So, below, line1 will start on y16, line2 on y29, etc.
#define line1 16
#define line2 29
#define line3 42
#define line4 55

/**
 * @brief UI is the class that controls the display, the alarm, and the LEDs. It displays
 * "status info" on the top (yellow) line, and cycles through all of the other data 
 * the receiver is getting from the transmitters. This class also controls the Alarm,
 * which is used when any packet's data is "out of range", but could also be used to alert
 * of something like no wifi, or a failed web update. And it controls the LEDs.
 */

class UI {

private:
    Adafruit_SSD1306* display_ = NULL;
    Alarm* alarm_;
    Internet* net_ = NULL;
    uint8_t blue_led_pin_;
    uint8_t buzzer_pin_;

public:
    
    UI(uint8_t blue_led_pin, uint8_t buzzer_pin) : blue_led_pin_{blue_led_pin}, buzzer_pin_{buzzer_pin} {
        display_ = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
        alarm_ = new Alarm(buzzer_pin_);
        net_ = new Internet();
        pinMode(blue_led_pin, OUTPUT);
        digitalWrite(blue_led_pin, LOW);
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, LOW);
    }

    void prepare_display() {
        if (display_->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println("OLED successfully started");
        }
        else {
            Serial.println("SSD1306 allocation failed");
        }
        display_->clearDisplay();
        display_->setCursor(0,0);
        // If you don't do these next two lines, there is no display at all for some reason
        display_->drawPixel(0, 0, SSD1306_WHITE);
        display_->setTextColor(SSD1306_WHITE);
        display_about_screen();
    }

    /**
     * @brief Updates the bottom 4 lines of the display to show everything about a single
     * datapoint.
     */
     
   void display_one_packet(Packet_it_t packet) {
       clear_packet_area();
       display_->setTextSize(1);
       display_->setCursor(0, line1);
       display_->print(packet->data_source);
       display_->print("-");
       display_->println(packet->data_name);
       display_->setCursor(0, line2);
       display_->print(packet->data_value);
       // convert Age to a string of M:SS
       int32_t seconds = ((millis() - packet->timestamp) / 1000);
       char age_buffer[10];
       String age_str = " Age: ";
       if (seconds < 3600) { // less than 1 hour old
           uint8_t minutes = seconds / 60;
           seconds = seconds % 60;
           sprintf(age_buffer, "%01d:%02d", minutes, seconds);
           age_str = age_str + age_buffer; // now it's " Age: 1:28"
       }
       else {
           age_str = age_str + "> 1 hr"; // now it's " Age: > 1 hr"
       }
       // pad the space btwn data_value and "Age" to right-justify the age string
       uint8_t text_padding_size = 21 - age_str.length() - packet->data_value.length();
       for (uint8_t x = 0; x < text_padding_size; x++) {
            age_str = " " + age_str;
        }
       display_->print(age_str);
       display_->setCursor(0, line3);
       if (packet->alarm_code) {
           display_->print("  *** Alarm ");
           display_->print(packet->alarm_code);
           display_->print(" ***");
       }
       display_->setCursor(0, line4);
       if (packet->RSSI != 0) { // this is a packet from a transmitter
           display_->print("RSSI: ");
           display_->print(packet->RSSI);
           display_->print("  /  SNR: ");
           display_->print(packet->SNR);
       }
       display_->display();
       if (packet->alarm_code && !packet->alarm_has_sounded) {
           alarm_->sound_alarm(packet->alarm_code);
           packet->alarm_has_sounded = true;
       }
   }

   
   /**
    * @brief Uses the whole screen to display info about the program 
    */
   
   void display_about_screen() {
       clear_packet_area();
       display_->setTextSize(1);
       display_->setCursor(0, line1);
       display_->println("Boat Monitor");
       display_->setCursor(0, line2);
       display_->println("Ver 1.90");
       display_->setCursor(0, line3);
       display_->print("28 Apr, 2020");
       display_->display();
       update_status_line("JimBooth's", 5, 2);
   }

   
   /**
    * @brief Updates the yellow status line (the top line of the OLED)
    * 
    * @param status_str - The string you want to display there - max length is 21.
    * @param duration_seconds - # of seconds to display this string before the next update 
    * of the status line. If not specified, it's 1 second.
    */

   void update_status_line(String status_str, uint8_t duration_seconds = 1, uint8_t temp_font_size = 1) {
       clear_status_line();
       display_->setTextSize(temp_font_size);
       display_->print(status_str);
       display_->display();
       if (duration_seconds) {
           delay(duration_seconds * 1000);
       }
       display_->setTextSize(1);
   }

    /**
     * @brief Display the status just before connecting to wifi.
     */
    void before_connect_to_wifi_screen() {
        update_status_line("Connecting to:");
        update_status_line(net_->get_ssid());
    }

    /**
     * @brief Display the status just after connecting to wifi.
     */
    void after_connect_to_wifi_screen(String local_ip) {
        if (net_->connected_to_wifi()) {  
            update_status_line("Connected to:");
            update_status_line(local_ip, 3);
            update_status_line("Waiting for data", 0);
        }
        else { // not connected
           update_status_line("Wait for data:no wifi");
        }
    }

    
    /**
     * @brief Clears the top yellow line of the OLED and prepares it for the next text
     * to display there.
     */
    
    void clear_status_line() {
        display_->setCursor(0, 0);
        // This is how Jim did it, and it works
        for (int y = 0; y <= 15; y++) {
            for (int x = 0; x < 127; x++) {
                display_->drawPixel(x, y, BLACK);
            }
        }
        // drawFastHLine causes a crash   
        // display_->drawFastHLine(0, 15, 15, SSD1306_BLACK);
        display_->setCursor(0, 0);
        display_->display();
    }

    void clear_packet_area() {
        for (int y = line1; y <= SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                display_->drawPixel(x, y, BLACK);
            }
        }
        // this crashes the system
        // display_->drawFastHLine(line1, 63, 48, SSD1306_BLACK);
        display_->display();
    }

    /**
     * @brief Make a very brief "beep" to alert of something about to happen,
     * like a new packet being processed.
     */

    void beep(uint8_t duration = 5) {
        alarm_->soundAlarm(duration, 1);
    }

    void turnOnLed() {
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(blue_led_pin_, HIGH);
    }

    void turnOFFLed() {
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(blue_led_pin_, LOW);
    }

}; // class UI

#endif // _UI_H_
