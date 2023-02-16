#pragma once

#include "Utils.hpp"

enum LightEffect : unsigned char{
  NORMAL,
  FADE,
  SLIDE
};

class ILightEffect{

    public:

      LightEffect effect;
      State currentState;
      unsigned int lightOffAfter;

      Adafruit_NeoPixel& neopixels;
      std::vector<PixelStep>& pixels;

      bool _isDirty;

      ILightEffect(Adafruit_NeoPixel& neopixels_,std::vector<PixelStep>& pixels_):neopixels(neopixels_),pixels(pixels_),currentState(State::NONE){
      }

      bool isDirty(){
        return _isDirty;
      }

      void setState(State currentState_){
        if(currentState==NONE){
            currentState=currentState_;
        }
        onStateChange();
      }

      RGBW computeGradient(RGBW color1,RGBW color2, float percent){
          
          float r1=float(color1.r)/255.f;
          float g1=float(color1.g)/255.f;
          float b1=float(color1.b)/255.f;

          float r2=float(color2.r)/255.f;
          float g2=float(color2.g)/255.f;
          float b2=float(color2.b)/255.f;

          RGBW f;
          f.r = (r1 + percent * (r2 - r1))*255.f;
          f.g = (g1 + percent * (g2 - g1))*255.f;
          f.b = (b1 + percent * (b2 - b1))*255.f;
          //f.b = (b1 + percent * (b2 - b2))*255.f;
          return f;
      }

      virtual void onStateChange() = 0;

};
