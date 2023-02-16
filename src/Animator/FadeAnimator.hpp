#pragma once

#include "IAnimator.hpp"

// multiple color in this mode
class FadeAnimator : public IAnimator{

        std::vector< std::array<unsigned char,4> > colors;
        unsigned int speed;

        std::array<unsigned char,4> colorTransition(unsigned int i){
            std::array<unsigned char,4> color;
            return color;
        }
    public :

        FadeAnimator(JsonObject& object){
            int error = 0;
            error = getValueFromJSON<int, unsigned int>(object["speed"] | -1 , speed, -1);
            if (error) {
                Serial.println("FadeAnimator speed error parsing");
                speed=0;
            }
        }

        void run(Adafruit_NeoPixel* neopixels, std::vector<PixelStep>& pixels){

            // std::array<unsigned char,4> color = colorTransition(0);

            // unsigned int iter = colors.size()/pixels.size();
            // for(unsigned int j=0;j<iter;j++){
            //     for(unsigned int i=pixels.begin();i<pixels.end();i++){
            //         neopixels->setPixelColor(i, color[0], color[1], color[2]);
            //     }
            //     neopixels->show();
            //     color = colorTransition(j);
            //     delay(30);
            // }
        }

};