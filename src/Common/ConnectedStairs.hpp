#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <array>
#include <sstream>

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h> 

#include "Utils.hpp"
#include "NeoPixel.hpp"
#include "Stepxel.hpp"

#include "ConnectedStairsState.hpp"
#include "LightEffect/ILightEffect.hpp"
#include "LightEffect/StaticEffect.hpp"

enum Mode{
  SMART,
  ALEXA,
  WEB
};

class ConnectedStairs {

    NeoPixel _pixelsDriver;
    Mode _mode;
    unsigned int _LDRThreshold;
    bool _LDR;
    std::vector<Stepxel> _stepxels;
    std::shared_ptr<ILightEffect> _animator;

    bool _isConfigure;

    std::shared_ptr<ILightEffect> animatorBuilder(const JsonObject& object){
        
        // TODO: add lighterEffect
        bool error;
        unsigned int numericEffect;
        error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(object["type"] | -1 , numericEffect, -1);
        if (!error) {
            Serial.println(F("lightEffect::type error parsing"));
            return std::make_shared<StaticEffect>(_pixelsDriver,_stepxels,object);
        }

      // int numericEffect = 0;
      // switch(numericEffect){
      //     case 0:
      //         return std::make_shared<NormalAnimator>(object);
      //     // case 1:
      //     //     return std::make_shared<FadeAnimator>(object);
      //     // case 2:
      //     //     return std::make_shared<SlideAnimator<L2R>>(object);
      //     // case 3:
      //     //     return std::make_shared<SlideAnimator<R2L>>(object);
      //     // case 4:
      //     //     return std::make_shared<SlideAnimator<C2E>>(object);
      //     // case 5:
      //     //     return std::make_shared<SlideAnimator<E2C>>(object);

      // }
      // return NULL;
       return std::make_shared<StaticEffect>(_pixelsDriver,_stepxels,object);
    }

  public:

    ConnectedStairs():_mode(Mode::SMART),_LDRThreshold(0),_LDR(false),_isConfigure(false){}

    bool getStatus(){
      return _isConfigure;
    }

    void onUp2Down();

    void onDown2Up();

    void lightOn(const RGBW& color);

    void lightOff();

    void onLDR(unsigned int value);

    bool deserialize(const JsonObject& root);

    bool loadFromMemory(const char* json);

    bool loadFromFile(String confPath="/stairsConf.json");

    void saveToFile(String confPath="/stairsConf.json");

    void changeManagerMode(Mode value){_mode = value;}

    void setBrightness(unsigned char value){
        _pixelsDriver.setBrightness(value);
    }


};

