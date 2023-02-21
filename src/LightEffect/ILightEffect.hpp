#pragma once

#include <vector>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <Ticker.h>

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
        bool _isOn;
        unsigned int _speed;
        bool _isDirty;
        unsigned int _startTimer;
        unsigned int _resetTimer;
        int _maxExecutionCount;
        Ticker _threadOff;

        ILightEffect(NeoPixel& pixelsDriver,std::vector<Stepxel>& stepxels,const JsonObject& object):
        _pixelsDriver(pixelsDriver),_stepxels(stepxels),_isOn(false),_currentState(ConnectedStairsState::OFF){
            _matrix.clear();

             _isDirty = false;
            bool error = false;
            error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(object["speed"] | -1 , _speed, -1);
            if(!error) {
                Serial.println("StaticEffect::speed error parsing");
                _isDirty=true;
            }


            error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(object["lightOffAfter"] | -1 , _lightOffAfter, -1);
            if(!error) {
                Serial.println("StaticEffect::lightOffAfter error parsing");
                _isDirty=true;
            }

            const JsonArray& maxGradientArray = object["matrixGradient"];
            
            for (JsonVariant arrayMaxGradient : maxGradientArray) {
                float y;
                error = ArduinoJson::extends::getValueFromJSON<double, float>(arrayMaxGradient["at"] |  -1.0 , y, -1.0);
                if (!error) {
                    Serial.println(F("at error parsing")); 
                    float t = arrayMaxGradient["at"];
                    Serial.println(t);
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
                //TODO float map?
                _matrix.addLine(y*10,lineGradient);
   
            }

            if(_isDirty){
                defaultConfig();
            }
            _matrix.buildGradientSquare();
            _maxExecutionCount=_stepxels.size()+1;

 ColorStopSquareView squareView;
            _matrix.getColorStopSquareView(0.2,0.2,squareView);
            _matrix.getColorStopSquareView(0.7,0.7,squareView);

        }

        void defaultConfig(){
            _speed= 300;
            _lightOffAfter = 5000;
        }

        bool isDirty(){
            return _isDirty;
        }

        void resetStepxelStatus(){
            for(unsigned int i=0;i< _stepxels.size();i++){
                _stepxels[i].setStatus(false);
            }
        }

        bool getLightStatus(){
            return _isOn;
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
                            // Serial.println("off2off");
                            _currentState=state;
                        break;
                        case ConnectedStairsState::ON:
                            // Serial.println("off");
                            _currentState=state;
                            lightOff(true);
                        break;
                        case ConnectedStairsState::DOWN2UP_FINISHED:
                            // Serial.println("status off");
                            _currentState = state;
                        break;
                        case ConnectedStairsState::UP2DOWN_FINISHED:
                            // Serial.println("status off");
                            _currentState = state;
                        break;
                    }
                    break;

                case ConnectedStairsState::ON:
                    switch(_currentState){
                        case ConnectedStairsState::ON:
                            //Serial.println("on2on");
                            _currentState=state;
                            lightOn(true,color);
                        break;
                        case ConnectedStairsState::OFF:
                            //Serial.println("on");
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
            float yy = float(y)/(_stepxels.size()-1);
            float xx = float(x)/(_stepxels[y].size()-1);

            // Serial.print("xx :");Serial.print(xx);
            // Serial.print(" yy :");Serial.println(yy);

            if(xx<0.f) xx=0.f;
            if(xx>1.f) xx=1.f; 

            if(yy<0.f) yy=0.f;
            if(yy>1.f) yy=1.f;

            ColorStopSquareView squareView;
            bool err = _matrix.getColorStopSquareView(xx,yy,squareView);

            if(err){
                return bilinearInterpolateColor(squareView.at(0)->getColor(),squareView.at(1)->getColor(),squareView.at(2)->getColor(),squareView.at(3)->getColor(),xx,yy );
            }

            Serial.print("errrrrr");
            return RGBW(0,0,0,0);
        }

        virtual void interrupt() = 0;

        void lightOn(bool isNow=false, const RGBW& color=RGBW(0,0,0,0)){

            _isOn = true;

            if(isNow){

                interrupt();
               
                if(color.isBlack()){

                    for(int y=0; y <_stepxels.size();y++){
                        //Serial.print("stepY : ");Serial.println(y);
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
                    _pixelsDriver.display(color);
                }
                if(_currentState!=ConnectedStairsState::ON)
                    setState(ConnectedStairsState::ON);

            }else{
                _resetTimer= _startTimer = millis();

                // executionCount=0;
                _maxExecutionCount=_stepxels.size()+1;

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

         void lightOff(bool isNow = false){
            Serial.println("lightOff");
            _isOn = false;
            if(isNow){
                _pixelsDriver.display(RGBW(0,0,0,0));
                resetStepxelStatus();
                setState(ConnectedStairsState::OFF);
            }else{
               // Serial.println("lightOff in " + (_lightOffAfter+_resetTimer-_startTimer));
                _threadOff.detach();
                _threadOff.once_ms(_lightOffAfter+_resetTimer-_startTimer,[&](){
                    _pixelsDriver.display(RGBW(0,0,0,0));
                    resetStepxelStatus();
                    setState(ConnectedStairsState::OFF);
                });
            }
        }

        void rebootTimer(){
            _resetTimer = millis();
            lightOff(1);
        }      

      void serialize(JsonObject& object){
            object["type"] = 0;
		    object["speed"] = _speed;
		    object["lightOffAfter"] = _lightOffAfter;

            JsonArray maxGradientArray = object.createNestedArray("matrixGradient");
            for(auto it : _matrix.getRawData()){
                
                JsonObject r = maxGradientArray.createNestedObject();
                r["at"] = float(it.first)/10.0;
                JsonArray l = r.createNestedArray("lineGradient");
                for(auto it2 : it.second){
                    JsonObject u = l.createNestedObject();
                    u["at"] = it2.getX();
                    u["color"] = it2.getColor();
                }
            }
        };

        virtual void lightUp2Down() = 0;
        virtual void lightDown2Up() = 0;


};
