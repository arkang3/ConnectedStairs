#pragma once
#include <Arduino.h>
#include <sstream>

class RGBW{

    unsigned char _r;
    unsigned char _g;
    unsigned char _b;
    unsigned char _w;

    public:

        static String FullBlack;

        RGBW(){
            _r = 0;
            _g = 0;  
            _b = 0;  
            _w = 0;  
        }

        RGBW(unsigned char r, unsigned char g, unsigned char b, unsigned char w){
            _r = r;
            _g = g;  
            _b = b; 
            _w = w; 
        }

        RGBW(String color){
            unsigned int hexValue;
            std::stringstream sstream;
            sstream << std::hex << color.c_str();
            sstream >> hexValue;
            _r = ((hexValue >> 16) & 0xFF) ;// Extract the GG byte
            _g = ((hexValue >> 8) & 0xFF) ;  
            _b = ((hexValue) & 0xFF) ;  
        }

        RGBW(unsigned int color){
            _r = ((color >> 16) & 0xFF) ;
            _g = ((color >> 8) & 0xFF) ;  
            _b = ((color) & 0xFF) ;  
        }

        RGBW(const RGBW& color) = default;

        String toHex(){
            std::ostringstream oss;
            oss << "0x";
            oss.fill('0');
            oss.width(6);
            oss << std::uppercase << std::hex << ((_r << 24) | (_g << 16) | (_b << 16) | _w);
            return String(oss.str().c_str());
        }

        uint32_t pack() const{
              return (((uint32_t)_w << 24) | ((uint32_t)_r << 16) | ((uint32_t)_g << 8) | _b);
        }

        bool isBlack() const{
            if(_r == 0 && _g == 0 && _b == 0 && _w == 0)
                return true;
            return false;
        }

        unsigned char getR() const{ return _r;}
        unsigned char getG() const{ return _g;}
        unsigned char getB() const{ return _b;}
        unsigned char getW() const{ return _w;}

};

