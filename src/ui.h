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
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on
    digitalWrite(blueLED, HIGH);
}

void turnOFFLed() {
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
    Adafruit_SSD1306* display_;
    Alarm* alarm_;

public:
    
    UI() {
        Adafruit_SSD1306* display_ = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
        alarm_ = new Alarm(buzzerPin);
        if (!display_->begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
            Serial.println("SSD1306 allocation failed");
        }
    }

    void prepare_display() {
        display_->clearDisplay();
        display_->display();
        delay(2000);

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
    
   // BAS: remove all the lines below that print static text instead of real data 
   void display_one_packet(Packet_t packet) {
       display_->clearDisplay();
       display_->setTextSize(1);
       display_->setCursor(0, line1);
       //display_->print(packet.data_source);
       display_->print("Bessie");
       display_->print(": ");
       //display_->println(packet.data_name);
       display_->println("Voltage");
       display_->setCursor(0, line2);
       //display_->print(packet.data_value);
       display_->print("15.99");
       display_->print(" Age: ");
       // BAS: make this display more than just seconds if there's room
       //display_->println((millis() - packet.timestamp) / 1000);
       display_->println("1234");
       display_->setCursor(0, line3);
       display_->print("RSSI/SNR: ");
       //display_->print(packet.RSSI);
       display_->print("-99");
       display_->print("/");
       //display_->print(packet.SNR);
       display_->print("88");
       display_->display();
       if (packet.alarm_code && !packet.alarm_has_sounded) {
           alarm_->sound_alarm(packet.alarm_code);
           packet.alarm_has_sounded = true;
       }
   }

    /**
     * @brief does with the OLED what used to be just before ConnecToWifi() in the old program
     */
    void before_connect_to_wifi_screen() {
        display_->setCursor(0,0);
        display_->print(F("Connecting to: "));
        display_->setCursor(0, 20);
        display_->setTextSize(1);
        display_->println(F(SSID));
        display_->display();
        display_->setTextSize(1);
    }

    /**
     * @brief - does with the OLED what used to be just after ConnectToWifi()
     */
    void after_connect_to_wifi_screen(String local_ip) {
        display_->clearDisplay();
        display_->setCursor(0, 0);
        display_->println(F("Connected to"));
        display_->setCursor(0, 30);
        display_->setTextSize(1);
        display_->println(local_ip);
        display_->display();
        delay(5000);
        display_->setTextSize(1);
        display_->clearDisplay();
        display_->setCursor(0, 0);
        display_->println(F("Awaiting data"));
        display_->display();
    }

    void clear_top_line() {
        // clears the info message at TOP of OLED
        // BAS: I think this should be done with display.drawFastHLine(0, 15, 15), then display.display().
            /*
            for (int y = 0; y <= 15; y++) {
                for (int x = 0; x < 127; x++) {
                    display_->drawPixel(x, y, BLACK);
                }
            }
            */
            display_->drawFastHLine(0, 15, 15, SSD1306_BLACK);
            display_->display();
    }
    

    // if there is data, this function is called
    // BAS: this will be replaced by the concept of a separate display page for each
    // item of data we receive.
    // updateOLED(line1, Bes1, battery1)
    void updateOLED(int start_line, String transmitter, String voltage, String RSSI) {
        //Serial.println("battery 1");
        display_->setTextSize(1);
        //display.drawPixel(0, line1, SSD1306_WHITE);
        for (int y = start_line; y <= start_line + 6; y++) {
            for (int x = 0; x < 127; x++) {
                display_->drawPixel(x, y, BLACK);
            }
        }
        display_->setCursor(0, start_line);
        // display.println(F("Bes1"));
        display_->println(transmitter);
        display_->setCursor(30, start_line);
        display_->println(voltage);
        display_->setCursor(65, start_line);
        display_->println(F("RSSI: "));
        display_->setCursor(95, start_line);
        display_->println(RSSI);
        display_->display();
    } 

}; // class UI

#endif // _UI_H_
