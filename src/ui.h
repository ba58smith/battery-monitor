#ifndef _UI_H_
#define _UI_H_

//#include "Arduino.h"
#include "config.h"
#include "packet_t.h"
#include "packet_list.h"
#include <Adafruit_SSD1327.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include "DejaVu_Sans_12.h"
#include "DejaVu_Sans_12_bold.h"
#include "alarm.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels
#define OLED_RESET -1

// a quick way to adjust the vertical seperation between the 
// text lines on the oled. So, below, line1 will start on y16, line2 on y29, etc.
#define line1 25
#define line2 39
#define line3 53
#define line4 67
#define line5 81
#define line6 95
#define line7 109
#define line8 123

/**
 * @brief UI is the class that controls the display, the alarm, and the LEDs. It displays
 * "status info" on the top (yellow) line, and cycles through all of the other data 
 * the receiver is getting from the transmitters. This class also controls the Alarm,
 * which is used when any packet's data is "out of range", but could also be used to alert
 * of something like no wifi, or a failed web update.
 */

class UI {

private:
    Adafruit_SSD1327* display_ = NULL;
    Alarm* alarm_;
    uint8_t blue_led_pin_;
    uint8_t buzzer_pin_;

public:
    
    UI(uint8_t blue_led_pin, uint8_t buzzer_pin) : blue_led_pin_{blue_led_pin}, buzzer_pin_{buzzer_pin} {
        display_ = new Adafruit_SSD1327(128, 128, &Wire, OLED_RESET, 1000000);
        alarm_ = new Alarm(buzzer_pin_);
        pinMode(blue_led_pin, OUTPUT);
        digitalWrite(blue_led_pin, LOW);
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, LOW);
    }

    void prepare_display() {
        if (display_->begin(0x3D)) { // 0x3D if DC wire is connected to VCC, or 0x3C if connected to GND
            Serial.println("OLED successfully started");
        }
        else {
            Serial.println("SSD1327 allocation failed");
        }
        display_->clearDisplay();
        display_->setCursor(0,0);
        // If you don't do these next two lines, there is no display at all for some reason
        display_->setTextColor(SSD1327_WHITE);
        display_->setFont(&DejaVu_Sans_12);
        display_about_screen();
    }

    /**
     * @brief Updates the bottom 4 lines of the display to show everything about a single
     * datapoint.
     */
     
    void display_one_packet(Packet_it_t packet) {
       clear_packet_area();
       display_->setCursor(0, line4);
       display_->print(packet->data_source);
       display_->print("-");
       display_->println(packet->data_name);
       display_->setCursor(0, line5);
       display_->print(packet->data_value);
       // convert Age to a string of M:SS
       int32_t seconds = ((millis() - packet->timestamp) / 1000);
       char age_buffer[10];
       String age_str = "Age: ";
       if (seconds < 3600) { // less than 1 hour old
           uint8_t minutes = seconds / 60;
           seconds = seconds % 60;
           sprintf(age_buffer, "%01d:%02d", minutes, seconds);
           age_str = age_str + age_buffer; // now it's " Age: 1:28"
       }
       else {
           age_str = age_str + ">1 hr"; // now it's "Age: >1 hr"
       }
       // pad the space btwn data_value and "Age" to right-justify the age string
       uint8_t text_padding_size = 17 - age_str.length() - packet->data_value.length();
       for (uint8_t x = 0; x < text_padding_size; x++) {
            age_str = " " + age_str;
        }
       display_->print(age_str);
       display_->setCursor(0, line6);
       if (packet->alarm_code) {
           display_->print(" ** Alarm ");
           display_->print(packet->alarm_code);
           display_->print(" **");
       }
       display_->display();
       if (packet->alarm_code && !packet->alarm_has_sounded) {
           alarm_->sound_alarm(packet->alarm_code);
           packet->alarm_has_sounded = true;
       }
    }
   
    /**
     * @brief Displays info about the program 
    */
   
    void display_about_screen() {
       update_status_lines("Jim Booth's", "Boat Monitor");
       clear_packet_area();
       display_->println("Version 2.40");
       display_->setCursor(0, line4);
       display_->print("11 Nov, 2022");
       display_->display();
       delay(5000);
       clear_status_area();
       clear_packet_area();

    }
   
    /**
    * @brief Updates the top two line of the OLED
    * 
    * @param status_str - The string you want to display on the top line - max length is 21.
    * @param status_str2 - The string to display on the second line.
    * @param duration_seconds - # of seconds to display this string before the next update 
    * of the status line. If not specified, it's 1 second.
    */

    void update_status_lines(String status_str, String status_str2, uint8_t duration_seconds = 1, uint8_t temp_font_size = 1) {
       clear_status_area();
       display_->setTextSize(temp_font_size);
       display_->println(status_str);
       display_->print(status_str2);
       display_->display();
       if (duration_seconds) {
           delay(duration_seconds * 1000);
       }
       display_->setTextSize(1);
    }

    /**
     * @brief Update the display of the very bottom line
     * 
     * @param bottom_line_str - the string to display
     */

    void update_bottom_line(String bottom_line_str) {
        clear_bottom_line();
        display_->print(bottom_line_str);
        display_->display();
    }

    /**
     * @brief Display the status just before connecting to wifi.
     */
    void before_connect_to_wifi_screen(String ssid) {
        update_status_lines("Connect to:", ssid);
    }

    /**
     * @brief Display the status just after connecting to wifi.
     */
    void after_connect_to_wifi_screen(bool connected_to_wifi, String local_ip) {
        if (connected_to_wifi) {  
            update_status_lines("Connected to:", local_ip, 3);
            update_status_lines("Waiting for data", "", 0);
        }
        else { // not connected
           update_status_lines("Waiting for data", "(No wifi)");
        }
    }
    
    /**
     * @brief Clears the top two lines of the OLED.
     */
    void clear_status_area() {
        display_->setCursor(0, 0); // BAS: necessary?
        // This is how Jim did it, and it works
        for (int y = 0; y <= line3 - 1; y++) {
            for (int x = 0; x < 127; x++) {
                display_->drawPixel(x, y, SSD1327_BLACK);
            }
        }
        // drawFastHLine causes a crash   
        // display_->drawFastHLine(0, 15, 15, SSD1306_BLACK);
        display_->setCursor(0, line1); // Ready to print on the first line
        display_->display();
    }

    /**
     * @brief Clear everything BETWEEN the status lines and the bottom line,
     * then position the cursor to start printing in that area.
     * 
     */
    void clear_packet_area() {
        for (int y = line2 + 1; y < SCREEN_HEIGHT - 15; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                display_->drawPixel(x, y, SSD1327_BLACK);
            }
        }
        // this crashes the system
        // display_->drawFastHLine(line1, 63, 48, SSD1306_BLACK);
        display_->setCursor(0, line3);
        display_->display();
    }

    /**
     * @brief Clear just the bottom line and set the cursor to the
     * beginning of that line, ready to print.
     * 
     */
    void clear_bottom_line() {
        for (int y = SCREEN_HEIGHT - 14; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                display_->drawPixel(x, y, SSD1327_BLACK);
            }
        }
        display_->setCursor(0, line8);
        display_->display();

    }

    /**
     * @brief Make a very brief "beep" to alert of something about to happen,
     * like a new packet being processed.
     */

    void beep(uint8_t duration = 5) {
        alarm_->soundAlarm(duration, 1);
    }

    /**
     * @brief makes alarm_.sound_alarm() available outside the class
     */

    void sound_alarm(uint16_t alarm_code) {
        alarm_->sound_alarm(alarm_code);
    }

    /**
    * @brief Create a string from a tm struct. If a tm struct
    * is not provided, use current time (Mth dd @ HH:MM am/pm)
    */

    String date_time_str(tm* tm_to_convert = NULL) {
       struct tm timeinfo;
       if (tm_to_convert == NULL) {
           if (!getLocalTime(&timeinfo)) {
               Serial.println("Failed to obtain time");
               return "Invalid sys time";
           }
       }
       else {
           timeinfo = *tm_to_convert;
       }
       char time_buf[21];
       //strftime(time_buf, sizeof(time_buf), "%b %d: %I:%M:%S %P", &timeinfo); // BAS: this is too long - make it 2 lines?
       strftime(time_buf, sizeof(time_buf), "%b %d  %I:%M %P", &timeinfo); // this version has no seconds
       String current_time_string = time_buf;
       return current_time_string;
    }

    /**
     * @brief Make sure system clock is set. Use any time a timestamp is created or
     * compared to.
     */

    bool system_time_is_valid() {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
               Serial.println("Failed to obtain time");
               update_status_lines("Invalid sys time", "", 3);
               return false;
        }
        char year_buf[5];
        strftime(year_buf, sizeof(year_buf), "%G", &timeinfo);
        String year_string = year_buf;
        uint16_t year_as_int = year_string.toInt();
        if (year_as_int > 2021) {
            return true;
        }
        else {
            return false;
        }
    }


    /**
     * @brief Display the system time on the status line. Call this
     * method in main.cpp on some kind of timer.
     */

    void display_system_time() {
        if (system_time_is_valid()) {
            Serial.println(date_time_str());
            update_bottom_line(date_time_str());
        }
        else {
            Serial.println("Invalid system time");
            update_bottom_line("Invalid sys time");
        }
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
