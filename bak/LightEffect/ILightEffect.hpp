#pragma once

#include <vector>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <Ticker.h>

#include "../Common/NeoPixel.hpp"
#include "../Common/Stepxel.hpp"
#include "ColorStopMatrix.hpp"

enum LightEffect : unsigned char{
  NORMAL,
  FADE,
  SLIDE
};

class ILightEffect{

    public:

        NeoPixel& _pixelsDriver;
        std::vector<Stepxel>& _stepxels;
        ColorStopMatrix<RGB>& _matrix;

        LightEffect _effect;
        unsigned int _speed;
        bool _isDirty;
        unsigned short _maxExecutionCount;

        ILightEffect(NeoPixel& pixelsDriver,std::vector<Stepxel>& stepxels,ColorStopMatrix<RGB>& matrix,const JsonObject& object):
        _pixelsDriver(pixelsDriver),_stepxels(stepxels),_matrix(matrix){
            
            _isDirty = false;
            bool error = false;
            error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(object["speed"] | -1 , _speed, -1);
            if(!error) {
                Serial.println("StaticEffect::speed error parsing");
                _isDirty=true;
            }

            if(_isDirty){
                defaultConfig();
            }
            
            _maxExecutionCount =_stepxels.size()+1;
            
        }

        void defaultConfig(){
            _speed = 300;
        }

        bool isDirty(){
            return _isDirty;
        }

        void serialize(JsonObject& object){
		    object["speed"] = _speed;
        };

        virtual void lightUp2Down() = 0;
        virtual void lightDown2Up() = 0;
        virtual void interrupt() = 0;
        virtual void setStateFunc(std::function<void(ConnectedStairsState)> setState) = 0;


};
