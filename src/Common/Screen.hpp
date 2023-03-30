#pragma once

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <memory>
#include <map>
#include <thread>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

class Screen{

        static Adafruit_SSD1306 _oledScreen ;
        static std::thread _thDisplayer;
        static std::map< std::pair<uint8_t,uint8_t>, String > _multiline;

    public:

        static void begin(int8_t switchAPVCC, int8_t screenAdress, int SDA, int SCL){

            Wire.begin(SDA,SCL);

            if(!_oledScreen.begin(switchAPVCC, screenAdress)) {
                Serial.println(F("SSD1306 allocation failed"));
                for(;;); // Don't proceed, loop forever
            }

            // Clear the buffer
            _oledScreen.display();
            delay(2000);
            _oledScreen.clearDisplay();
            _oledScreen.setTextSize(1);
            _oledScreen.setTextColor(SSD1306_WHITE);
            //_thDisplayer =  std::thread(displayy);
        }

        void setFontSize(uint8_t size){
            _oledScreen.setTextSize(size);
        }

        void setFontColor(int color){
            _oledScreen.setTextColor(color);
        }

        static void display(uint8_t x, uint8_t y, String text){
            _oledScreen.clearDisplay();
            _oledScreen.setCursor(x,y);             // Start at top-left corner
            _oledScreen.print(text);
            _oledScreen.display();
        }

        static void display(String text){
            _oledScreen.print(text);
            _oledScreen.display();
        }

        static void display(const std::map< std::pair<uint8_t,uint8_t>, String >& multiline){
            _oledScreen.clearDisplay();
            for(auto it : multiline){
                auto p = it.first;
                _oledScreen.setCursor(p.first,p.second); 
                _oledScreen.print(it.second);
            }
            
            _oledScreen.display();
        }

        static void displayy(){

            _oledScreen.clearDisplay();
            for(auto it : _multiline){
                auto p = it.first;
                _oledScreen.setCursor(p.first,p.second); 
                _oledScreen.print(it.second);
            }
            
            _oledScreen.display();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

        }

};

