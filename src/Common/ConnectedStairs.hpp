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
// #include "LightEffect/StaticEffect.hpp"
// #include "LightEffect/SlideEffect.hpp"
#include "LightEffect/FadeEffect.hpp"


class ConnectedStairs {

    NeoPixel _pixelsDriver;
    unsigned int _LDRThreshold;
    bool _LDR;
    std::vector<Stepxel> _stepxels;
    std::shared_ptr<ILightEffect> _lightEffect;
    bool _isConfigure;
    
    bool _isOn;
    unsigned int _lightOffAfter;
    unsigned int _speed;

    ConnectedStairsState _currentState;
    unsigned int _startTimer;
    unsigned int _resetTimer;

    ColorStopMatrix _matrix;

    Ticker _threadOff;
    Ticker _threadFade;
    float _fadeLight;
    unsigned int _fadems;
    RGBW _requestColor;

    std::function<void(ConnectedStairsState)> _ptrSetState;

    std::shared_ptr<ILightEffect> lightEffectBuilder(const JsonObject& object){
        
        bool error;
        unsigned int numericEffect;
        error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(object["type"] | -1 , numericEffect, -1);
        if (!error) {
            Serial.println(F("lightEffect::type error parsing"));
            return std::make_shared<FadeEffect>(_pixelsDriver,_stepxels,_matrix,object);
        }

      // int numericEffect = 0;
      // switch(numericEffect){
      //     case 0:
      //         return std::make_shared<StaticEffect>(_pixelsDriver,_stepxels,object);
      //     // case 1:
      //     //     return std::make_shared<SlideEffect>(_pixelsDriver,_stepxels,object);
      //     // case 2:
      //     //     return std::make_shared<FadeEffect>(_pixelsDriver,_stepxels,object);
      //     // case 3:
      //     //     return std::make_shared<SlideAnimator<R2L>>(object);
      //     // case 4:
      //     //     return std::make_shared<SlideAnimator<C2E>>(object);
      //     // case 5:
      //     //     return std::make_shared<SlideAnimator<E2C>>(object);

      // }
      // return NULL;
       return std::make_shared<FadeEffect>(_pixelsDriver,_stepxels,_matrix,object);
    }

  public:

    ConnectedStairs():_LDRThreshold(0),_LDR(false),_isConfigure(false),_currentState(ConnectedStairsState::OFF),_fadems(20){
      _ptrSetState = std::bind(&ConnectedStairs::setState, this, std::placeholders::_1,RGBW(0,0,0,0));
    }

    bool getStatus(){
      return _isOn;
    }

    bool getConfigurationStatus(){
      return _isConfigure;
    }

    void onUp2Down();

    void onDown2Up();

    void onlightOn(const RGBW& color);

    void onlightOff();

    void onLDR(unsigned int value);

    bool deserialize(const JsonObject& root);

    bool loadFromMemory(const char* json);

    bool loadFromFile(String confPath="/stairsConf.json");

    void saveToFile(String confPath="/stairsConf.json");

    void setBrightness(unsigned char value){
        _pixelsDriver.setBrightness(value);
    }

    void resetStepxelStatus();
    void setState(ConnectedStairsState state,const RGBW& color = RGBW(0,0,0,0));

    void rebootTimer();
    void lightOn(bool isNow=false, const RGBW& color=RGBW(0,0,0,0));
    void lightOff(bool isNow = false);


};

