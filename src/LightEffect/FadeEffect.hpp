#pragma once

#include <mutex>
#include <ArduinoJson.h>

#include "../Common/NeoPixel.hpp"
#include "../Common/Stepxel.hpp"
#include "../Common/Utils.hpp"

#include "ILightEffect.hpp"

class FadeEffect : public ILightEffect{

        Ticker _threadOnDown2Up;
        Ticker _threadOnUp2Down;
        bool _isInterrupt;

        unsigned int _executionCountOnDown2Up;
        unsigned int _executionCountOnUp2Down;

    public:

        FadeEffect(NeoPixel& pixelsDriver,std::vector<Stepxel>& stepxels,ColorStopMatrix& matrix,const JsonObject& object):ILightEffect(pixelsDriver,stepxels,matrix,object){
            _maxExecutionCount=_stepxels.size()+1;
            _isInterrupt=false;
            Serial.println("FadeEffect");
        }

        void serialize(JsonObject& object){
            object["type"] = 0;
            ILightEffect::serialize(object);
        };

        void lightDown2Up(std::function<void(ConnectedStairsState)> setState){
            Serial.println("lightDown2Up");
            _executionCountOnDown2Up=0;
            _threadOnDown2Up.attach_ms(_speed,[&](){

                //TODO
                setState(ConnectedStairsState::DOWN2UP_RUNNING);

                _executionCountOnDown2Up++;

                if(!_isInterrupt){

                    if(_executionCountOnDown2Up>=_maxExecutionCount){

                        _threadOnDown2Up.detach();
                        Serial.print("waiting....");

                        setState(ConnectedStairsState::DOWN2UP_FINISHED);

                    }else{
                        unsigned int y = _executionCountOnDown2Up-1;
                        const Stepxel& stepxels = _stepxels[y];

                        if(_executionCountOnDown2Up<=2){
                            std::function<RGBW(unsigned int)> colorFunc = [&](unsigned int x){
                                float yy = float(y)/(_stepxels.size()-1);
                                float xx = float(x)/(_stepxels[y].size()-1);
                                RGBW requestColor = _matrix.getGradientColor(xx,yy);
                                return requestColor;
                            };
                            _pixelsDriver.display(stepxels.begin(),stepxels.size(),colorFunc);
                            const_cast<Stepxel&>(stepxels).setStatus(true);
                        }else{
                            std::function<RGBW(unsigned int, RGBW, float)> colorFunc = [&](unsigned int x,RGBW currrentColor, float brightness){
                                float yy = float(y)/(_stepxels.size()-1);
                                float xx = float(x)/(_stepxels[y].size()-1);
                                RGBW requestColor = _matrix.getGradientColor(xx,yy);
                                unsigned char r = currrentColor.getRedColor<int>()*brightness + requestColor.getRedColor<int>()*(1-brightness);
                                unsigned char g = currrentColor.getGreenColor<int>()*brightness + requestColor.getGreenColor<int>()*(1-brightness);
                                unsigned char b = currrentColor.getBlueColor<int>()*brightness + requestColor.getBlueColor<int>()*(1-brightness);
                                unsigned char w = currrentColor.getWhiteColor<int>()*brightness + requestColor.getWhiteColor<int>()*(1-brightness);
                                RGBW color(r,g,b,w);
                                return color;
                            };

                            if(!stepxels.status()){
                                for(float brightness=1;brightness>=0;brightness-=0.05){
                                    _pixelsDriver.display(stepxels.begin(),stepxels.size(),colorFunc,brightness);
                                    delay(20);
                                }
                                const_cast<Stepxel&>(stepxels).setStatus(true);
                            }
                        }
                    }
                }else{
                    _isInterrupt=false;
                    _executionCountOnDown2Up=_maxExecutionCount;
                    _threadOnDown2Up.detach();
                    Serial.print("force stop....");
                }
                
            });
        }

        void lightUp2Down(std::function<void(ConnectedStairsState)> setState){
            Serial.println("lightUp2Down");
            _executionCountOnUp2Down=0;

            _threadOnUp2Down.attach_ms(_speed,[&](){

                setState(ConnectedStairsState::UP2DOWN_RUNNING);

                _executionCountOnUp2Down++;

                if(!_isInterrupt){

                    if(_executionCountOnUp2Down>=_maxExecutionCount){

                        _threadOnUp2Down.detach();
                        Serial.println("waiting....");

                        setState(ConnectedStairsState::UP2DOWN_FINISHED);

                    }else{
                        unsigned int y = _executionCountOnUp2Down-1;
                        y=(_stepxels.size()-1)-(y);
                        const Stepxel& stepxels = _stepxels[y];

                         if(_executionCountOnDown2Up<=2){
                            std::function<RGBW(unsigned int)> colorFunc = [&](unsigned int x){
                                float yy = float(y)/(_stepxels.size()-1);
                                float xx = float(x)/(_stepxels[y].size()-1);
                                RGBW requestColor = _matrix.getGradientColor(xx,yy);
                                return requestColor;
                            };
                            _pixelsDriver.display(stepxels.begin(),stepxels.size(),colorFunc);
                            const_cast<Stepxel&>(stepxels).setStatus(true);
                        }else{
                            std::function<RGBW(unsigned int, RGBW, float)> colorFunc = [&](unsigned int x,RGBW currrentColor, float brightness){
                                float yy = float(y)/(_stepxels.size()-1);
                                float xx = float(x)/(_stepxels[y].size()-1);
                                RGBW requestColor = _matrix.getGradientColor(xx,yy);
                                unsigned char r = currrentColor.getRedColor<int>()*brightness + requestColor.getRedColor<int>()*(1-brightness);
                                unsigned char g = currrentColor.getGreenColor<int>()*brightness + requestColor.getGreenColor<int>()*(1-brightness);
                                unsigned char b = currrentColor.getBlueColor<int>()*brightness + requestColor.getBlueColor<int>()*(1-brightness);
                                unsigned char w = currrentColor.getWhiteColor<int>()*brightness + requestColor.getWhiteColor<int>()*(1-brightness);
                                RGBW color(r,g,b,w);
                                return color;
                            };

                            if(!stepxels.status()){
                                for(float brightness=1;brightness>=0;brightness-=0.05){
                                    _pixelsDriver.display(stepxels.begin(),stepxels.size(),colorFunc,brightness);
                                    delay(20);
                                }
                                const_cast<Stepxel&>(stepxels).setStatus(true);
                            }
                        }
                    }
                }else{
                    _isInterrupt=false;
                    _executionCountOnUp2Down=_maxExecutionCount;
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