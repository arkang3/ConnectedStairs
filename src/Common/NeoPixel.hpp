#pragma once

#include <map>
#include <Adafruit_NeoPixel.h>
#include "RGBW.hpp"
#include "Utils.hpp"

class NeoPixel{

        std::shared_ptr<Adafruit_NeoPixel> _neopixels;
        unsigned int _pin;
        String _strPin;
        uint16_t _neoPixeltype;
        String _strNeoPixeltype;
        unsigned int _size;
        unsigned char _brightness;
        unsigned char _maxBrightness;
        std::map<String,unsigned int> _pinMap;
        std::map<String,unsigned int> _dataTransmiterMap;
        std::map<String,unsigned int> _dataFreq;
        bool _isDirty;

       
    private:

        void defaultConfig(){
            configure("D8", "NEO_GRB+NEO_KHZ800" ,10,10,35);
        }

        void convertToPin(){
            auto it = _pinMap.find(_strPin);
            if(it != _pinMap.end()){
                _pin = it->second;
            }else{
                _pin = D8;
            }
        }

        void convertToType(){

            String dataTransmiter;
            String dataFreq;

            int n = sscanf(_strNeoPixeltype.c_str(), "%s %s", &dataTransmiter, &dataFreq);

            //if()
                auto it = _dataTransmiterMap.find(dataTransmiter);
                if(it != _dataTransmiterMap.end()){
                    _neoPixeltype = it->second;
                }else{
                    _neoPixeltype = NEO_GRB;
                }

                auto it2 = _dataFreq.find(dataFreq);
                if(it2 != _dataFreq.end()){
                    _neoPixeltype += it2->second;
                }else{
                    _neoPixeltype += NEO_KHZ800;
                }
            //}
        }


    public:

        NeoPixel():_isDirty(false){
            // TODO : best ?
            _pinMap["D0"] = D0;
            _pinMap["D1"] = D1;
            _pinMap["D2"] = D2;
            _pinMap["D3"] = D3;
            _pinMap["D4"] = D4;
            _pinMap["D5"] = D5;
            _pinMap["D6"] = D6;
            _pinMap["D7"] = D7;
            _pinMap["D8"] = D8;
            _pinMap["D9"] = D9;
            _pinMap["D10"] = D10;
            _pinMap["D11"] = D11;
            _pinMap["D12"] = D12;
            _pinMap["D13"] = D13;
            _pinMap["D14"] = D14;
            _pinMap["D15"] = D15;

            _dataTransmiterMap["NEO_RGB"] = NEO_RGB;
            _dataTransmiterMap["NEO_RBG"] = NEO_RBG;
            _dataTransmiterMap["NEO_GRB"] = NEO_GRB;
            _dataTransmiterMap["NEO_GBR"] = NEO_GBR;
            _dataTransmiterMap["NEO_BRG"] = NEO_BRG;
            _dataTransmiterMap["NEO_BGR"] = NEO_BGR;
            _dataTransmiterMap["NEO_WRGB"] = NEO_WRGB;
            _dataTransmiterMap["NEO_WRBG"] = NEO_WRBG;
            _dataTransmiterMap["NEO_WGRB"] = NEO_WGRB;
            _dataTransmiterMap["NEO_WGBR"] = NEO_WGBR;
            _dataTransmiterMap["NEO_WBRG"] = NEO_WBRG;
            _dataTransmiterMap["NEO_WBGR"] = NEO_WBGR;
            _dataTransmiterMap["NEO_RWGB"] = NEO_RWGB;
            _dataTransmiterMap["NEO_RWBG"] = NEO_RWBG;
            _dataTransmiterMap["NEO_RGWB"] = NEO_RGWB;
            _dataTransmiterMap["NEO_RGBW"] = NEO_RGBW;
            _dataTransmiterMap["NEO_RBWG"] = NEO_RBWG;
            _dataTransmiterMap["NEO_RBGW"] = NEO_RBGW;
            _dataTransmiterMap["NEO_GWRB"] = NEO_GWRB;
            _dataTransmiterMap["NEO_GWBR"] = NEO_GWBR;
            _dataTransmiterMap["NEO_GRWB"] = NEO_GRWB;
            _dataTransmiterMap["NEO_GRBW"] = NEO_GRBW;
            _dataTransmiterMap["NEO_GBWR"] = NEO_GBWR;
            _dataTransmiterMap["NEO_GBRW"] = NEO_GBRW;
            _dataTransmiterMap["NEO_BWRG"] = NEO_BWRG;
            _dataTransmiterMap["NEO_BWGR"] = NEO_BWGR;
            _dataTransmiterMap["NEO_BRWG"] = NEO_BRWG;
            _dataTransmiterMap["NEO_BRGW"] = NEO_BRGW;
            _dataTransmiterMap["NEO_BGWR"] = NEO_BGWR;
            _dataTransmiterMap["NEO_BGRW"] = NEO_BGRW;

            _dataFreq["NEO_KHZ800"] = NEO_KHZ800;
            _dataFreq["NEO_KHZ400"] = NEO_KHZ400;
        }

        bool isDirty(){
            return _isDirty;
        }

        void configure(const JsonObject& object, unsigned int size){
            
            bool error=false;
            _isDirty=false;
            String pin = "D8";
            error = ArduinoJson::extends::getValueFromJSON<const char*, String>(object["pin"] | ((const char*)(NULL)) , pin, ((const char*)(NULL)));
            if (!error) {
                Serial.println(F("NeoPixel::pin error parsing"));
                _isDirty = true;
            }
            unsigned char brightness = 35;
            error = ArduinoJson::extends::getValueFromJSON<int, unsigned char>(object["brightness"] | -1 , brightness, -1);
            if (!error) {
                Serial.println(F("NeoPixel::brightness error parsing"));
                _isDirty = true;
            }
            unsigned char maxBrightness = 35;
            error = ArduinoJson::extends::getValueFromJSON<int, unsigned char>(object["maxBrightness"] | -1 , maxBrightness, -1);
            if (!error) {
                Serial.println(F("NeoPixel::maxBrightness error parsing"));
                _isDirty = true;
            }
            String neoPixeltype;
            error = ArduinoJson::extends::getValueFromJSON<const char*, String>(object["type"] | ((const char*)(NULL)) , neoPixeltype, ((const char*)(NULL)));
            if (!error) {
                Serial.println(F("NeoPixel::type error parsing"));
                _isDirty = true;
            }

            if(_isDirty){
                defaultConfig();
            }else{
                configure(pin,neoPixeltype,size,brightness,maxBrightness);
            }
         
        }

        void configure(String pin, String neoPixeltype, unsigned int size, unsigned char brightness, unsigned char maxBrightness){
            
            _strPin = pin;
            convertToPin();
            _strNeoPixeltype = neoPixeltype;
            convertToType();
            _size = size;
            _brightness = brightness;
            _maxBrightness = maxBrightness;

            Serial.println("Neopixel: ");
            Serial.print("    _size: ");
            Serial.println(_size);
            Serial.print("    _pin: ");
            Serial.println(_strPin);
            Serial.print("    _type: ");
            Serial.println(_strNeoPixeltype);
            Serial.print("    _brightness: ");
            Serial.println(_brightness);
            Serial.print("    _maxBrightness: ");
            Serial.println(_maxBrightness);
    
            if(_brightness>_maxBrightness){
                _brightness = _maxBrightness;
            }
            _neopixels.reset(new Adafruit_NeoPixel(_size,_pin,NEO_GRB  + NEO_KHZ800));
            _neopixels->begin();
            _neopixels->clear();
            _neopixels->setBrightness(_brightness);
            _neopixels->show();
        }

        void serialize(JsonObject& object){
            object["pin"]= _strPin;
            object["brightness"]= _brightness;
            object["maxBrightness"]= _maxBrightness;
            object["type"]= _strNeoPixeltype;
        }

        unsigned char getBrighnessMax(){
            return _maxBrightness;
        }

        void setBrightness(unsigned int value){
            if(_brightness!=value && value <=_maxBrightness){
                _neopixels->setBrightness(value);
                _neopixels->show();
                _brightness = value;
            }
        }

        void clear() {
            _neopixels->clear();
            _neopixels->show();
        }

        void display(){
            _neopixels->show();
        }

         void display(const RGBW& color){
             _neopixels->fill(color.getColor(),0);
             _neopixels->show();
        }

        void display(unsigned int begin, unsigned int size, const RGBW& color){
             _neopixels->fill(color.getColor(),begin,size);
             _neopixels->show();
        }

        void display(unsigned int begin, unsigned int size, std::function<RGBW(unsigned int)> func){
            unsigned int end = begin+size; 
            unsigned int xx=0;
            for(auto x=begin;x<end;x++){
                _neopixels->setPixelColor(x,func(xx).getColor());
                xx++;
            }
            _neopixels->show();
        }

        void setColorLine(unsigned int begin, unsigned int end, const RGBW& color){
            for(auto i=begin;i<end;i++){
                _neopixels->setPixelColor(i,color.getColor());
            }
        }

        void setColorLine(unsigned int begin, unsigned int end, const std::vector<RGBW>& color){
            for(auto i=begin;i<end;i++)
                _neopixels->setPixelColor(i,color[i].getColor());
        }

        void setColorLine(unsigned int begin, unsigned int end, std::function<RGBW(unsigned int)> func){
            for(auto x=begin;x<end;x++){
                _neopixels->setPixelColor(x,func(x).getColor());
            }
        }



};