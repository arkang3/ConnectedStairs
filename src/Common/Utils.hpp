#pragma once

#include <sstream>
#include "../colorModel/RGB.hpp"
#include "../colorModel/RGBW.hpp"

namespace ArduinoJson{
    namespace extends{
        template<class T, class U>
        inline bool getValueFromJSON(T jsonValue, U& value, T errorValue) {
            if (jsonValue == errorValue)
                return false;
            U t(jsonValue);
            value = t;
            return true;
        }

        inline bool strToJson(const char* json,DynamicJsonDocument& doc){
            
            DeserializationError DesError = deserializeJson(doc, json);
            if(DesError) {
                Serial.println(F("Failed to parse config file"));
                Serial.println(DesError.f_str());
                return false;
            }
            return true;
        }

        inline double round2(double value) {
            return (int)(value * 100 + 0.5) / 100.0;
        }
    }
}

namespace FastMath{

    inline uint8_t blend8(uint8_t a, uint8_t b, uint8_t amountOfB){
        uint16_t partial;
        uint8_t result;

        uint8_t amountOfA = 255 - amountOfB;

        partial = (a * amountOfA);
        partial += a;

        partial += (b * amountOfB);
        partial += b;

        result = partial >> 8;

        return result;
    }

    inline uint8_t blend8by0(uint8_t a, uint8_t amountOfB){
        uint16_t partial;
        uint8_t result;

        uint8_t amountOfA = 255 - amountOfB;

        partial = (a * amountOfA);
        partial += a;

        result = partial >> 8;

        return result;
    }

    typedef uint8_t fract8;

    inline uint8_t scale8(uint8_t i, fract8 scale)
    {
        return (((uint16_t) i) * (1 + (uint16_t) (scale))) >> 8;
    }

    inline uint8_t lerp8by8(uint8_t a, uint8_t b, fract8 frac)
    {
        uint8_t result;
        if (b > a)
        {
            uint8_t delta = b - a;
            uint8_t scaled = scale8(delta, frac);
            result = a + scaled;
        }
        else
        {
            uint8_t delta = a - b;
            uint8_t scaled = scale8(delta, frac);
            result = a - scaled;
        }
        return result;
    }

    inline RGB blend8(const RGB& color1,const RGB& color2, uint8_t amountOfB){
        auto r = blend8(color1.getR(),color2.getR(),amountOfB);
        auto g = blend8(color1.getG(),color2.getG(),amountOfB);
        auto b = blend8(color1.getB(),color2.getB(),amountOfB);
        return RGB(r,g,b);
    }

    inline RGB blend8by0(const RGB& color, uint8_t amountOfB){
        auto r = blend8by0(color.getR(),amountOfB);
        auto g = blend8by0(color.getG(),amountOfB);
        auto b = blend8by0(color.getB(),amountOfB);
        return RGB(r,g,b);
    }

    inline RGB lerp8by8(const RGB& c1, const RGB& c2, fract8 frac){
        auto r = FastMath::lerp8by8(c1.getR(),c2.getR(),frac);
        auto g = FastMath::lerp8by8(c1.getG(),c2.getG(),frac);
        auto b = FastMath::lerp8by8(c1.getB(),c2.getB(),frac);
        return RGB(r,g,b);
    }

    inline RGBW blend8(const RGBW& color1,const RGBW& color2, uint8_t amountOfB){
        auto r = blend8(color1.getR(),color2.getR(),amountOfB);
        auto g = blend8(color1.getG(),color2.getG(),amountOfB);
        auto b = blend8(color1.getB(),color2.getB(),amountOfB);
        auto w = blend8(color1.getW(),color2.getW(),amountOfB);
        return RGBW(r,g,b,w);
    }

    inline RGBW blend8by0(const RGBW& color, uint8_t amountOfB){
        auto r = blend8by0(color.getR(),amountOfB);
        auto g = blend8by0(color.getG(),amountOfB);
        auto b = blend8by0(color.getB(),amountOfB);
        auto w = blend8by0(color.getW(),amountOfB);
        return RGBW(r,g,b,w);
    }

    inline RGBW lerp8by8(const RGBW& c1, const RGBW& c2, fract8 frac){
        auto r = FastMath::lerp8by8(c1.getR(),c2.getR(),frac);
        auto g = FastMath::lerp8by8(c1.getG(),c2.getG(),frac);
        auto b = FastMath::lerp8by8(c1.getB(),c2.getB(),frac);
        auto w = FastMath::lerp8by8(c1.getW(),c2.getW(),frac);
        return RGBW(r,g,b,w);
    }


}


template<class T,class U>
inline U convertToNumeric(T& data){
    U value;
    std::stringstream sstream;
    sstream << data.c_str();
    sstream >> value;
    return value;
}

