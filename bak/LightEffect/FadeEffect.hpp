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

        unsigned short _executionCountOnDown2Up;
        unsigned short _executionCountOnUp2Down;

        std::function<void(ConnectedStairsState)> _setState;

    public:

        FadeEffect(NeoPixel& pixelsDriver,std::vector<Stepxel>& stepxels,ColorStopMatrix<RGB>& matrix,const JsonObject& object):ILightEffect(pixelsDriver,stepxels,matrix,object){
            _maxExecutionCount=_stepxels.size()+1;
            _isInterrupt=false;
            Serial.println(F("FadeEffect"));
        }

        void serialize(JsonObject& object){
            object["type"] = 0;
            ILightEffect::serialize(object);
        };

        void setStateFunc(std::function<void(ConnectedStairsState)> setState){
            _setState=setState;
        }

        void lightDown2Up(){
            Serial.println("lightDown2Up");
            _executionCountOnDown2Up=0;
            _threadOnDown2Up.attach_ms(_speed,[&](){

                //TODO
                _setState(ConnectedStairsState::DOWN2UP_RUNNING);

                _executionCountOnDown2Up++;

                if(!_isInterrupt){

                    if(_executionCountOnDown2Up>=_maxExecutionCount){

                        // Serial.println("waiting....");
                        _threadOnDown2Up.detach();
                       _setState(ConnectedStairsState::DOWN2UP_FINISHED);

                    }else{
                        unsigned int y = _executionCountOnDown2Up-1;
                        const Stepxel& stepxel = _stepxels[y];

                        for(float _fadeLight=1;_fadeLight>=0;_fadeLight-=0.05){
                            float yy = float(y)/float(_stepxels.size()-1);
                            for(unsigned int x=0;x <stepxel.size();x++ ){
                                float xx = float(x)/float(stepxel.size()-1);
                                RGB requestColor = _matrix.getGradientColor(xx,yy);
                                RGB currrentColor = _pixelsDriver.getPixel(stepxel.begin()+x);
                                RGB ncolor = FastMath::blend8(currrentColor,requestColor,_fadeLight);
                                _pixelsDriver.setPixel(stepxel.begin()+x,ncolor.pack());
                                _stepxels[y].setStatus(true);
                            }
                        }
                        _pixelsDriver.display();
                    }
                }else{
                    // Serial.println("force stop....");
                    _isInterrupt=false;
                    _executionCountOnDown2Up=_maxExecutionCount;
                    _threadOnDown2Up.detach();
                }
                
            });
        }

        void lightUp2Down(){
            // Serial.println("lightUp2Down");
            // TODO: refaire les indices
            _executionCountOnUp2Down=0;

            _threadOnUp2Down.attach_ms(_speed,[&](){

                _setState(ConnectedStairsState::UP2DOWN_RUNNING);

                _executionCountOnUp2Down++;

                if(!_isInterrupt){

                    if(_executionCountOnUp2Down>=_maxExecutionCount){

                        _threadOnUp2Down.detach();
                        // Serial.println("waiting....");

                        _setState(ConnectedStairsState::UP2DOWN_FINISHED);

                    }else{
                        unsigned int y = _executionCountOnUp2Down-1;
                        y=(_stepxels.size()-1)-(y);
                        const Stepxel& stepxel = _stepxels[y];

                        for(float _fadeLight=1;_fadeLight>=0;_fadeLight-=0.05){
                            float yy = float(y)/float(_stepxels.size()-1);
                            for(unsigned int x=0;x <stepxel.size();x++ ){
                                float xx = float(x)/float(stepxel.size()-1);
                                RGB requestColor = _matrix.getGradientColor(xx,yy);
                                RGB currrentColor = _pixelsDriver.getPixel(stepxel.begin()+x);
                                RGB ncolor = FastMath::blend8(currrentColor,requestColor,_fadeLight);
                                _pixelsDriver.setPixel(stepxel.begin()+x,ncolor.pack());
                                _stepxels[y].setStatus(true);
                            }
                        }
                        _pixelsDriver.display();
                    }
                }else{
                    _isInterrupt=false;
                    _executionCountOnUp2Down=_maxExecutionCount;
                    _threadOnUp2Down.detach();
                    // Serial.print("force stop....");
                }
                
            });
        }

        void interrupt(){
            if(_threadOnDown2Up.active() || _threadOnUp2Down.active())
                _isInterrupt=true;
        }

};