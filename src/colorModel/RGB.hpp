#pragma once
#include <Arduino.h>
#include <sstream>

class RGB{

    uint8_t _r;
    uint8_t _g;
    uint8_t _b;

    public:

        static String FullBlack;

        RGB(){
            _r = 0;
            _g = 0;  
            _b = 0;  
        }

        RGB(uint8_t r, uint8_t g, uint8_t b){
            _r = r;
            _g = g;  
            _b = b;  
        }

        RGB(String color){
            uint32_t hexValue;
            std::stringstream sstream;
            sstream << std::hex << color.c_str();
            sstream >> hexValue;
            _r = ((hexValue >> 16) & 0xFF) ;// Extract the GG byte
            _g = ((hexValue >> 8) & 0xFF) ;  
            _b = ((hexValue) & 0xFF) ;  
        }

        RGB(uint32_t color){
            _r = ((color >> 16) & 0xFF) ;
            _g = ((color >> 8) & 0xFF) ;  
            _b = ((color) & 0xFF) ;  
        }

        RGB(const RGB& color) = default;

        String toHex() const {
            std::ostringstream oss;
            oss << "0x";
            oss.fill('0');
            oss.width(6);
            oss << std::uppercase << std::hex << ((_r << 16) | (_g << 8) | _b);
            return String(oss.str().c_str());
        }

        uint32_t pack() const{
              return (((uint32_t)_r << 16) | ((uint32_t)_g << 8) | _b);
        }

        bool isBlack() const{
            if(_r == 0 && _g == 0 && _b == 0)
                return true;
            return false;
        }

        uint8_t getR() const{ return _r;}
        uint8_t getG() const{ return _g;}
        uint8_t getB() const{ return _b;}

};

