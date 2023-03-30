#include "Screen.hpp"
#include <thread>

Adafruit_SSD1306 Screen::_oledScreen = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
std::thread Screen::_thDisplayer;
std::map< std::pair<uint8_t,uint8_t>, String > Screen::_multiline;