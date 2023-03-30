#pragma once

#include <map>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <memory>
#include <mutex>

#include "../common/Utils.hpp"

class NeoPixel{

     public:

        enum ColorModel{
            RGB,
            RGBW
        };
    
    private:

        std::shared_ptr<Adafruit_NeoPixel> _neopixels;
        uint16_t _pin;
        uint16_t _neoPixeltype;
        String _strNeoPixeltype;
        uint16_t _size;
        uint8_t _brightness;
        uint8_t _maxBrightness;
        //TODO: change type
        std::map<String,uint32_t> _dataTransmiterMap;
        std::map<String,uint32_t> _dataFreq;
        ColorModel _colorModel = ColorModel::RGBW;

        std::mutex _driverMutex;


    private:

        void convertToType();


    public:

        NeoPixel();

        static String confPath(){
            return "/neoPixelConf.json";
        }
    
        ColorModel getColorModel();

        bool loadFromFile();
        bool loadFromMemory(const char* json);
        void saveToFile();

        bool deserialize(const JsonObject& object);

        void updateLength(uint16_t size);

        uint8_t getBrightnessMax();
        uint8_t getBrightness();
        void setBrightness(const uint8_t& value);

        void clear();

        void display();
        void display(const uint32_t& color);
        void display(const uint16_t& begin, const uint16_t& size, const uint32_t& color);

        uint32_t getPixel(const uint16_t& position);

        void setPixel(const uint16_t& position, const uint32_t& color);
        void setPixel(const uint16_t& position, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& w);
        void setPixel(const uint16_t&position, const uint8_t& r, const uint8_t& g, const uint8_t& b);
    
};