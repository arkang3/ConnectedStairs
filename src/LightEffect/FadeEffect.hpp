#pragma once

#include <mutex>
#include <Ticker.h>
#include <ArduinoJson.h>

#include "../Common/NeoPixel.hpp"
#include "../Common/Stepxel.hpp"
#include "../Common/Utils.hpp"

#include "ILightEffect.hpp"

class FadeEffect : public ILightEffect{

        unsigned int _speed;
        unsigned int _startTimer;
        unsigned int _resetTimer;

        Ticker _threadOnDown2Up;
        Ticker _threadOnUp2Down;
        Ticker _threadFade;
        Ticker _threadOff;

        bool _isInterrupt;

        int executionCount;
        int maxExecutionCount;

        void defaultConfig(){
            _speed= 300;
            _lightOffAfter = 5000;
        }

    public:

        FadeEffect(NeoPixel& pixelsDriver,std::vector<Stepxel>& stepxels,const JsonObject& object):ILightEffect(pixelsDriver,stepxels){
            _isDirty = false;
            bool error = false;
            error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(object["speed"] | -1 , _speed, -1);
            if(!error) {
                Serial.println("FadeEffect::speed error parsing");
                _isDirty=true;
            }


            error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(object["lightOffAfter"] | -1 , _lightOffAfter, -1);
            if(!error) {
                Serial.println("FadeEffect::lightOffAfter error parsing");
                _isDirty=true;
            }

            const JsonArray& maxGradientArray = object["matrixGradient"];
            
            for (JsonVariant arrayMaxGradient : maxGradientArray) {
                float y;
                error = ArduinoJson::extends::getValueFromJSON<float, float>(arrayMaxGradient["at"] |  -1 , y, -1);
                if (!error) {
                    Serial.print(F("at error parsing"));
                    _isDirty = true;
                }

                //at == y
 
                std::vector<ColorStop> lineGradient;
                const JsonArray& lineGradientArray = arrayMaxGradient["lineGradient"];
                for (JsonVariant arrayLineGradient : lineGradientArray) {
                    float x;
                    error = ArduinoJson::extends::getValueFromJSON<float,float>(arrayLineGradient["at"] | -1 , x, -1);
                    if(!error) {
                        _isDirty=true;
                    }
                    String color;
                    error = ArduinoJson::extends::getValueFromJSON<const char*, String>(arrayLineGradient["color"] | ((const char*)(NULL)) , color, ((const char*)(NULL)));
                    if(!error) {
                        _isDirty=true;
                    }

                    lineGradient.emplace_back(x,y,color);
                }

                _matrix.addLine(y,lineGradient);
   
            }

            if(_isDirty){
                defaultConfig();
            }

            // Serial.print("speed : ");
            // Serial.println(_speed);
            // Serial.print("lightOffAfter : ");
            // Serial.println(_lightOffAfter);

            _isInterrupt=false;
            _matrix.buildGradientSquare();
            RGBW xm = getGradientColor(1,20);

            // Serial.print("gradient : ");
            // Serial.print(xm.getRedColor<int>());Serial.print(" ");
            // Serial.print(xm.getGreenColor<int>());Serial.print(" ");
            // Serial.print(xm.getBlueColor<int>());Serial.print(" ");
            // Serial.print(xm.getWhiteColor<int>());Serial.print(" ");
            // Serial.println(" ");
        }

        void rebootTimer(){
            _resetTimer = millis();
            lightOff(1);
        }       

        void lightOff(bool isNow){
            if(isNow){
                _pixelsDriver.display(RGBW(0,0,0,0));
                resetStepxelStatus();
                setState(ConnectedStairsState::OFF);
            }else{

                _threadOff.detach();
                _threadOff.once_ms(_lightOffAfter+_resetTimer-_startTimer,[&](){
                    _pixelsDriver.display(RGBW(0,0,0,0));
                    resetStepxelStatus();
                    setState(ConnectedStairsState::OFF);
                });
            }
        }

        void lightDown2Up(){

            _threadOnDown2Up.attach_ms(_speed,[&](){

                setState(ConnectedStairsState::DOWN2UP_RUNNING);

                executionCount++;

                if(!_isInterrupt){

                    if(executionCount>=maxExecutionCount){

                        _threadOnDown2Up.detach();
                        Serial.print("waiting....");

                        setState(ConnectedStairsState::DOWN2UP_FINISHED);

                    }else{
                        unsigned int y = executionCount-1;
                        const Stepxel& stepxels = _stepxels[y];

                        std::function<RGBW(unsigned int)> colorFunc = [&](int x){
                            return getGradientColor(x,y);
                        };

                        if(!stepxels.status()){
                            _pixelsDriver.display(stepxels.begin(),stepxels.size(),colorFunc);
                            const_cast<Stepxel&>(stepxels).setStatus(true);
                        }
                    }
                }else{
                    _isInterrupt=false;
                    executionCount=maxExecutionCount;
                    _threadOnDown2Up.detach();
                    Serial.print("force stop....");
                }
                
            });
        }

        void lightUp2Down(){

            _threadOnUp2Down.attach_ms(_speed,[&](){

                setState(ConnectedStairsState::UP2DOWN_RUNNING);

                executionCount++;

                if(!_isInterrupt){

                    if(executionCount>=maxExecutionCount){

                        _threadOnUp2Down.detach();
                        Serial.print("waiting....");

                        setState(ConnectedStairsState::UP2DOWN_FINISHED);

                    }else{
                        unsigned int y = executionCount-1;
                        y=(_stepxels.size()-1)-(y);
                        const Stepxel& stepxels = _stepxels[y];

                        std::function<RGBW(unsigned int)> colorFunc = [&](int x){
                            return getGradientColor(x,y);
                        };

                        if(!stepxels.status()){
                            _pixelsDriver.display(stepxels.begin(),stepxels.size(),colorFunc);
                            const_cast<Stepxel&>(stepxels).setStatus(true);
                        }
                    }
                }else{
                    _isInterrupt=false;
                    executionCount=maxExecutionCount;
                    _threadOnUp2Down.detach();
                    Serial.print("force stop....");
                }
                
            });
        }

        void lightOn(bool isNow, const RGBW& color){

            if(isNow){

                if(_threadOnDown2Up.active() || _threadOnUp2Down.active())
                    _isInterrupt=true;
                
                if(color.isFullBlack()){

                    for(int y=0; y <_stepxels.size();y++){
                        const Stepxel& stepxels = _stepxels[y];

                        std::function<RGBW(unsigned int)> colorFunc = [&](int x){
                            return getGradientColor(x,y);
                        };

                        if(!stepxels.status()){
                            _pixelsDriver.setColorLine(stepxels.begin(),stepxels.size(),colorFunc);
                            const_cast<Stepxel&>(stepxels).setStatus(true);
                        }           
                    }
                    _pixelsDriver.display();
                    

                }else{
                    _pixelsDriver.display(color);
                }
                if(_currentState!=ConnectedStairsState::ON)
                    setState(ConnectedStairsState::ON);

            }else{
                _resetTimer= _startTimer = millis();

                executionCount=0;
                maxExecutionCount=_stepxels.size()+1;

                switch(_currentState){
                    case ConnectedStairsState::DOWN2UP_START:
                        lightDown2Up();
                    break;
                    case ConnectedStairsState::UP2DOWN_START:
                        lightUp2Down();
                    break;

                }
                
            }

        }

        void serialize(JsonObject& object){
            object["type"] = 0;
		    object["speed"] = _speed;
		    object["lightOffAfter"] = _lightOffAfter;

            JsonArray maxGradientArray = object.createNestedArray("matrixGradient");
            for(auto it : _matrix.getRawData()){
                
                JsonObject r = maxGradientArray.createNestedObject();
                r["at"] = it.first;
                JsonArray l = r.createNestedArray("lineGradient");
                for(auto it2 : it.second){
                    JsonObject u = l.createNestedObject();
                    u["at"] = it2.getX();
                    u["color"] = it2.getColor();
                }
                

            }

        };

};