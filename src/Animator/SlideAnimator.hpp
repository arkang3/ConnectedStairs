#pragma once

#include "IAnimator.hpp"

enum SLIDE_EFFECT{
    L2R,
    R2L,
    C2E,
    E2C
};

//only multiple colors in this mode
template<int R>
class SlideAnimator : public IAnimator{

    public :

        void run(Adafruit_NeoPixel* neopixels, PixelStep& pixels){}

};

//left to right
template<>
class SlideAnimator<L2R> : public IAnimator{

        unsigned int speed;
        std::vector< std::array<unsigned char,4> > colors;

    public :

        SlideAnimator<L2R>(JsonObject& object){
            int error = 0;
            error = getValueFromJSON<int, unsigned int>(object["speed"] | -1 , speed, -1);
            if (error) {
                Serial.println("SlideAnimator speed error parsing");
                speed=0;
            }
        }

        void run(Adafruit_NeoPixel* neopixels, std::vector<PixelStep>& pixels){
            // std::array<unsigned char,4> color;
            // for(unsigned int i=pixels.begin();i<pixels.end();i++){
            //     neopixels->setPixelColor(i, color[0], color[1], color[2]);
            //     neopixels->show();
            //     delay(30);
            // }
        }

};

// // right to left
// class SlideAnimator<R2L> : public IAnimator{

//     public :

//          SlideAnimator(JsonObject& object){
//             int error = 0;
//             error = getValueFromJSON<int, unsigned int>(object["speed"] | -1 , speed, -1);
//             if (error) {
//                 Serial.println("SlideAnimator speed error parsing");
//                 speed=0;
//             }
//         }

//         void run(Adafruit_NeoPixel* neopixels, PixelStep& pixels){
//             for(int i=end;i>start;i--){
//                 pixels->setPixelColor(i, color[0], color[1], color[2]);
//                 pixels->show();
//                 delay(30);
//             }
//         }

// };

// // center to extrem
// class SlideAnimator<C2E> : public IAnimator{

//     public :

//         SlideAnimator(JsonObject& object){
//             int error = 0;
//             error = getValueFromJSON<int, unsigned int>(object["speed"] | -1 , speed, -1);
//             if (error) {
//                 Serial.println("SlideAnimator speed error parsing");
//                 speed=0;
//             }
//         }

//         void run(Adafruit_NeoPixel* neopixels, PixelStep& pixels){}

// };

// // extrem to center
// class SlideAnimator<C2E> : public IAnimator{

//     public :

//         SlideAnimator(JsonObject& object){
//             int error = 0;
//             error = getValueFromJSON<int, unsigned int>(object["speed"] | -1 , speed, -1);
//             if (error) {
//                 Serial.println("SlideAnimator speed error parsing");
//                 speed=0;
//             }
//         }

//         void run(Adafruit_NeoPixel* neopixels, PixelStep& pixels){}

// };