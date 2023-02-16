#pragma once

#include "IAnimator.hpp"

//no color in this mode
class RainbowAnimator : public IAnimator{

        unsigned int speed;
        unsigned int transition;

    public :

        RainbowAnimator(JsonObject& object){
            int error = 0;
            error = getValueFromJSON<int, unsigned int>(object["speed"] | -1 , speed, -1);
            if (error) {
                Serial.println("RainbowAnimator speed error parsing");
                speed=0;
            }

        }

        void run(Adafruit_NeoPixel* neopixels, std::vector<PixelStep>& pixels){
            
            //unsigned int id = pixels.id();

            // for(unsigned int i=pixels.begin();i<pixels.end();i++){
            //     neopixels->setPixelColor(i, color[0], color[1], color[2]);
            // }
            // neopixels->show();
            // delay(speed);
        }

};