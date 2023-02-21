#pragma once

#include <mutex>
#include <ArduinoJson.h>

#include "../Common/NeoPixel.hpp"
#include "../Common/Stepxel.hpp"
#include "../Common/Utils.hpp"

#include "ILightEffect.hpp"

class SlideEffect : public ILightEffect{

        Ticker _threadOnDown2Up;
        Ticker _threadOnUp2Down;
        bool _isInterrupt;
        unsigned int _executionCount;
        
    public:

        SlideEffect(NeoPixel& pixelsDriver,std::vector<Stepxel>& stepxels,const JsonObject& object):ILightEffect(pixelsDriver,stepxels,object){
            _maxExecutionCount=_stepxels.size()+1;
            _isInterrupt=false;
        }

        void lightDown2Up(){
            Serial.print("lightDown2Up");
            _executionCount=0;
            _threadOnDown2Up.attach_ms(_speed,[&](){

                setState(ConnectedStairsState::DOWN2UP_RUNNING);

                _executionCount++;

                if(!_isInterrupt){

                    if(_executionCount>=_maxExecutionCount){

                        _threadOnDown2Up.detach();
                        Serial.print("waiting....");

                        setState(ConnectedStairsState::DOWN2UP_FINISHED);

                    }else{
                        unsigned int y = _executionCount-1;
                        const Stepxel& stepxels = _stepxels[y];

                        std::function<RGBW(unsigned int)> colorFunc = [&](int x){
                            return getGradientColor(x,y);
                        };

                        float wait = _speed/stepxels.size();

                        if(!stepxels.status()){
                            for(unsigned int t=0;t<stepxels.size();t++){
                                _pixelsDriver.display(stepxels.begin()+t,1,colorFunc);
                                delay(wait);
                            }
                            const_cast<Stepxel&>(stepxels).setStatus(true);
                        }
                    }
                }else{
                    _isInterrupt=false;
                    _executionCount=_maxExecutionCount;
                    _threadOnDown2Up.detach();
                    Serial.print("force stop....");
                }
                
            });
        }

        void lightUp2Down(){
            Serial.println("lightUp2Down");
            _executionCount=0;

            _threadOnUp2Down.attach_ms(_speed,[&](){

                setState(ConnectedStairsState::UP2DOWN_RUNNING);

                _executionCount++;

                if(!_isInterrupt){

                    if(_executionCount>=_maxExecutionCount){

                        _threadOnUp2Down.detach();
                        Serial.println("waiting....");

                        setState(ConnectedStairsState::UP2DOWN_FINISHED);

                    }else{
                        unsigned int y = _executionCount-1;
                        y=(_stepxels.size()-1)-(y);
                        const Stepxel& stepxels = _stepxels[y];

                        std::function<RGBW(unsigned int)> colorFunc = [&](int x){
                            return getGradientColor(x,y);
                        };

                        float wait = _speed/stepxels.size();

                        if(!stepxels.status()){
                            for(unsigned int t=0;t<stepxels.size();t++){
                                _pixelsDriver.display(stepxels.begin()+t,1,colorFunc);
                                delay(wait);
                            }
                            const_cast<Stepxel&>(stepxels).setStatus(true);
                        }
                    }
                }else{
                    _isInterrupt=false;
                    _executionCount=_maxExecutionCount;
                    _threadOnUp2Down.detach();
                    Serial.print("force stop....");
                }
                
            });
        }

        void interrupt(){
            if(_threadOnDown2Up.active() || _threadOnUp2Down.active())
                _isInterrupt=true;
        }

};