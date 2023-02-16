#pragma once

#include <map>
#include "ILightEffect.hpp"
#include <Ticker.h>

//only 1 color in this mode
class NormalAnimator : public ILightEffect{

        Ticker transitionTicker;
        int executionCount;
        int maxExecutionCount;

        Ticker offTicker;

        unsigned int startTimer;
        unsigned int resetTimer;

    private:

        unsigned int speed;
        unsigned int lightOffAfter;
        std::map<float,RGBW> colors;

    public :

        NormalAnimator(Adafruit_NeoPixel& neopixels_,std::vector<PixelStep>& pixels_,const JsonObject& object):
        ILightEffect(neopixels_,pixels_){
            _isDirty=false;
            bool error;
            error = getValueFromJSON<int, unsigned int>(object["speed"] | -1 , speed, -1);
            if(!error) {
                Serial.println("::speed error parsing");
                _isDirty=true;
            }
            error = getValueFromJSON<int, unsigned int>(object["lightOffAfter"] | -1 , lightOffAfter, -1);
            if(!error) {
                Serial.println("::lightOffAfter error parsing");
                _isDirty=true;
            }

            const JsonArray& array = object["colors"];
            String color;
            float at;
            for (JsonVariant arr : array) {

                error = getValueFromJSON<const char*, String>( arr["color"] , color, ((const char*)(NULL))) ;
                if (!error) {
                    Serial.print(F("color error parsing"));
                    _isDirty=true;
                } 
                error = getValueFromJSON<float, float>( arr["at"] | -1 , at, -1 ) ;
                if (!error) {
                    Serial.print(F("at error parsing"));
                    _isDirty=true;
                }

                colors.insert(std::make_pair(at,convertHexaToRGBW(color)));
            }

            for (auto it = colors.begin(); it != colors.end(); ++it) {
                Serial.print(it->first);
                Serial.print(", ");
                Serial.print(it->second.r);
                Serial.print("::");
                Serial.print(it->second.g);
                Serial.print("::");
                Serial.println(it->second.b);

            }

        }

        void onStateChange(){
            switch(currentState){
                case State::DOWN2UP_RUNNING:
                    Serial.print("restart.... ");
                    resetTimer = millis();
                    startOffTimer();
                    break;
                case State::DOWN2UP_START:
                    Serial.print("start.... ");
                    startLight();
                    break;
                case State::DOWN2UP_FINISHED:
                    Serial.print("finished.... ");
                    startOffTimer();
                    break;
                case State::UP2DOWN_RUNNING:
                    Serial.print("restart.... ");
                    resetTimer = millis();
                    startOffTimer();
                    break;
                case State::UP2DOWN_START:
                    Serial.print("start.... ");
                    startLight();
                    break;
                case State::UP2DOWN_FINISHED:
                    Serial.print("finished.... ");
                    startOffTimer();
                    break;

            }
        }

        void startOffTimer(){
            offTicker.detach();
            offTicker.once_ms(lightOffAfter+resetTimer-startTimer,[&](){
                        offLight(neopixels,pixels);
            });
        }

        void startLight(){
            resetTimer= startTimer = millis();

            executionCount=0;
            maxExecutionCount=pixels.size()+1;

            transitionTicker.attach_ms(speed,[&](){
                currentState=State::DOWN2UP_RUNNING;

                executionCount++;

                if(executionCount>=maxExecutionCount){
                    currentState=State::DOWN2UP_FINISHED;
                    transitionTicker.detach();
                    Serial.print("waiting....");
                    onStateChange();
                }else{
                    unsigned int id = executionCount-1;
                    unsigned int jId = id;
                    if(currentState==State::UP2DOWN_RUNNING)
                        jId=(pixels.size()-1)-(id);
                    const PixelStep& subpixels = pixels[jId];
                    float percent = float(id*100.f/(pixels.size()))/100.f;
                    RGBW color;// = computeGradient(startColorIn,endColorIn,percent);
                    // Serial.println(subpixels.begin());
                    // Serial.println(subpixels.end());
                    for(unsigned int i=subpixels.begin();i<subpixels.end();i++)
                        neopixels.setPixelColor(i, color.r,color.g,color.b);
                    
                    neopixels.show();
                }
            });

        }

        void offLight(Adafruit_NeoPixel& neopixels, std::vector<PixelStep>& pixels){
            Serial.print("runOut ");

            for(unsigned int i=0;i<pixels.size();i++){
                const PixelStep& subpixels = pixels[i];
                for(unsigned int i=subpixels.begin();i<subpixels.end();i++)
                    neopixels.setPixelColor(i, 0,0,0);
            }
            neopixels.show();
            currentState=State::NONE;
        }
};
