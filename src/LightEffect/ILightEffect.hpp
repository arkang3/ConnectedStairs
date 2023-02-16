#pragma once

#include <vector>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#include "../Common/NeoPixel.hpp"
#include "../Common/Stepxel.hpp"
#include "../Common/RGBW.hpp"
#include "../Common/ConnectedStairsState.hpp"
#include "ColorStopMatrix.hpp"

// #include "Utils.hpp"
// #include "Gradient.hpp"

enum LightEffect : unsigned char{
  NORMAL,
  FADE,
  SLIDE
};

class ILightEffect{

    public:

        NeoPixel& _pixelsDriver;
        std::vector<Stepxel>& _stepxels;
        ColorStopMatrix _matrix;
        ConnectedStairsState _currentState;
        unsigned int _lightOffAfter;
        LightEffect _effect;
        bool _isDirty;

        ILightEffect(NeoPixel& pixelsDriver,std::vector<Stepxel>& stepxels):
        _pixelsDriver(pixelsDriver),_stepxels(stepxels),_currentState(ConnectedStairsState::OFF){
            _matrix.clear();
        }

        bool isDirty(){
            return _isDirty;
        }

        void resetStepxelStatus(){
            for(unsigned int i=0;i< _stepxels.size();i++){
                _stepxels[i].setStatus(false);
            }
        }

        void setState(ConnectedStairsState state,const RGBW& color = RGBW(0,0,0,0)){

            switch(state){

                case ConnectedStairsState::UP2DOWN_START:
                    switch(_currentState){
                        case ConnectedStairsState::OFF: 
                            _currentState = state;
                            lightOn();
                            break;
                        case ConnectedStairsState::UP2DOWN_START:
                            //nothing todo
                            break;  
                        case ConnectedStairsState::UP2DOWN_RUNNING: 
                            Serial.println("restart UP2DOWN_RUNNING");
                            rebootTimer();
                            break;
                        case ConnectedStairsState::UP2DOWN_FINISHED: 
                            Serial.println("restart UP2DOWN_FINISHED");
                            rebootTimer();
                            break;  
                        case ConnectedStairsState::DOWN2UP_RUNNING: 
                            _currentState = state;
                            lightOn();
                            break;
                        case ConnectedStairsState::DOWN2UP_FINISHED: 
                            Serial.println("restart DOWN2UP_FINISHED");
                            rebootTimer();
                            break;
                    }
                    break;

                case ConnectedStairsState::DOWN2UP_START:
                    switch(_currentState){
                        case ConnectedStairsState::OFF: 
                            _currentState = state;
                            lightOn();
                            break;
                        case ConnectedStairsState::DOWN2UP_START:
                            //nothing todo
                            break;
                        case ConnectedStairsState::DOWN2UP_RUNNING: 
                            Serial.println("restart DOWN2UP_RUNNING");
                            rebootTimer();
                            break;
                        case ConnectedStairsState::DOWN2UP_FINISHED: 
                            Serial.println("restart DOWN2UP_FINISHED");
                            rebootTimer();
                            break;
                        
                    }
                    break;

                case ConnectedStairsState::DOWN2UP_RUNNING:
                    switch(_currentState){
                        case ConnectedStairsState::DOWN2UP_START:
                            _currentState = state;
                        break;
                        // case ConnectedStairsState::DOWN2UP_RUNNING:
                        //     _currentState = state;
                        // break;
                        // case ConnectedStairsState::DOWN2UP_FINISHED:
                        //     // _resetTimer = millis();
                        //     // startOffTimer();
                        // break;
                    }
                    break;

                case ConnectedStairsState::UP2DOWN_RUNNING:
                    switch(_currentState){
                        case ConnectedStairsState::UP2DOWN_START:
                            _currentState = state;
                        break;
                        // case ConnectedStairsState::DOWN2UP_RUNNING:
                        //     _currentState = state;
                        // break;
                        // case ConnectedStairsState::DOWN2UP_FINISHED:
                        //     // _resetTimer = millis();
                        //     // startOffTimer();
                        // break;
                    }
                    break;

                case ConnectedStairsState::DOWN2UP_FINISHED:
                    switch(_currentState){
                        // case ConnectedStairsState::DOWN2UP_START:
                        // break;
                        case ConnectedStairsState::DOWN2UP_RUNNING:
                            _currentState=state;
                            lightOff();
                        break;
                        // case ConnectedStairsState::DOWN2UP_FINISHED:
                        // break;
                    }
                    break;

                case ConnectedStairsState::UP2DOWN_FINISHED:
                    switch(_currentState){
                        // case ConnectedStairsState::DOWN2UP_START:
                        // break;
                        case ConnectedStairsState::UP2DOWN_RUNNING:
                            _currentState=state;
                            lightOff();
                        break;
                        // case ConnectedStairsState::DOWN2UP_FINISHED:
                        // break;
                    }
                    break;

                case ConnectedStairsState::OFF:
                    switch(_currentState){
                        case ConnectedStairsState::OFF:
                            Serial.println("off2off");
                            _currentState=state;
                        break;
                        case ConnectedStairsState::ON:
                            Serial.println("off");
                            _currentState=state;
                            lightOff(true);
                        break;
                        case ConnectedStairsState::DOWN2UP_FINISHED:
                            Serial.println("status off");
                            _currentState = state;
                        break;
                        case ConnectedStairsState::UP2DOWN_FINISHED:
                            Serial.println("status off");
                            _currentState = state;
                        break;
                    }
                    break;

                case ConnectedStairsState::ON:
                    switch(_currentState){
                        case ConnectedStairsState::ON:
                            Serial.println("on2on");
                            _currentState=state;
                            lightOn(true,color);
                        break;
                        case ConnectedStairsState::OFF:
                            Serial.println("on");
                            _currentState=state;
                            lightOn(true,color);
                        break;
                        case ConnectedStairsState::DOWN2UP_START:
                            _currentState=state;
                            lightOn(true,color);
                        break;
                        case ConnectedStairsState::DOWN2UP_RUNNING: 
                            _currentState=state;
                            lightOn(true,color);
                            break;
                         case ConnectedStairsState::UP2DOWN_START:
                            _currentState=state;
                            lightOn(true,color);
                        break;
                        case ConnectedStairsState::UP2DOWN_RUNNING: 
                            _currentState=state;
                            lightOn(true,color);
                            break;
                    }
                    break;

            }

        }

        RGBW interpolateColor(RGBW c1,RGBW c2,float fraction){

            float red1 = c1.getRedColor<float>()/255.f;
            float green1 = c1.getGreenColor<float>()/255.f;
            float blue1 = c1.getBlueColor<float>()/255.f;
            float white1 = c1.getWhiteColor<float>()/255.f;

            float red2 = c2.getRedColor<float>()/255.f;
            float green2 = c2.getGreenColor<float>()/255.f;
            float blue2 = c2.getBlueColor<float>()/255.f;
            float white2 = c2.getWhiteColor<float>()/255.f;

            float deltaRed = red2 - red1;
            float deltaGreen = green2 - green1;
            float deltaBlue = blue2 - blue1;
            float deltaWhite = white2 - white1;

            float red = red1 + (deltaRed * fraction);
            float green = green1 + (deltaGreen * fraction);
            float blue = blue1 + (deltaBlue * fraction);
            float white = white1 + (deltaWhite * fraction);

            if(red>1.f) red=1.f;
            if(red<0.f) red=0.f;

            if(green>1.f) green=1.f;
            if(green<0.f) green=0.f;

            if(blue>1.f) blue=1.f;
            if(blue<0.f) blue=0.f;

            if(white>1.f) white=1.f;
            if(white<0.f) white=0.f;

            return RGBW(red*255, green*255, blue*255, white*255);        
        }

        RGBW bilinearInterpolateColor(RGBW upperL,RGBW upperR, RGBW lowerL, RGBW lowerR, float x, float y){
            RGBW x1 = interpolateColor(upperL, upperR, x);
            RGBW x2 = interpolateColor(lowerL, lowerR, x);
            return interpolateColor(x1, x2, y);
            return RGBW();
        }

        RGBW getGradientColor(unsigned int x, unsigned int y){
            //TODO: test gradient complexe
            float yy = float(y)/_stepxels.size();
            float xx = float(x)/_stepxels[y].size();

            if(xx<0.f) xx=0.f;
            if(xx>1.f) xx=1.f; 

            if(yy<0.f) yy=0.f;
            if(yy>1.f) yy=1.f;

            ColorStopSquareView squareView;
            bool err = _matrix.getColorStopSquareView(xx,yy,squareView);

            if(err){
                //Serial.println(squareView.at(0)->getColor());
                return bilinearInterpolateColor(squareView.at(0)->getColor(),squareView.at(1)->getColor(),squareView.at(2)->getColor(),squareView.at(3)->getColor(),xx,yy );

            }

            Serial.print("errrrrr");
            return RGBW(0,0,0,0);
        }

      virtual void serialize(JsonObject& object) = 0;
      virtual void defaultConfig() = 0;
      virtual void lightOn(bool isNow=false,const RGBW& color=RGBW(0,0,0,0)) = 0;
      virtual void lightOff(bool isNow=false) = 0;
      virtual void rebootTimer() = 0;


};
