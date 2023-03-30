#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <array>
#include <sstream>
#include <thread>
#include <map>
#include <atomic>

#include <esp_pthread.h>

#include <ArduinoJson.h>
#include <StreamUtils.h>

#include "../common/Utils.hpp"
#include "../driver/NeoPixel.hpp"

#include "ColorStopMatrix.hpp"

#include "ConnectedStairsState.hpp"
#include "Stepxel.hpp"

#include "../fsStream/FSStreamInternal.hpp"
#include "../fsStream/FSStreamMode.hpp"

class IConnectedStairs{

  public:

    NeoPixel& _pixelsDriver;
    std::vector<Stepxel> _stepxels;

    uint32_t _LDRThreshold = 0;
    uint16_t _lightOffAfter;
    uint16_t _lightStepSpeed;

    bool _isConfigure = false;

    bool _LDR =false;
    std::atomic<bool> _isOn;


    ConnectedStairsState _currentState = ConnectedStairsState::OFF;

    uint16_t _fadems = 50;

    std::mutex _mutex;
    std::shared_ptr<std::thread> _thFadeOn;
    std::shared_ptr<std::thread> _thFadeOff;
    std::shared_ptr<std::thread> _thLightDown2Up;
    std::shared_ptr<std::thread> _thLightUp2Down;
    std::shared_ptr<std::thread> _thAnimateLight;
    std::atomic<bool> _thBreaker;

    IConnectedStairs(NeoPixel& pixelsDriver):_pixelsDriver(pixelsDriver){
        _isOn = false;
        _thBreaker=false;
        esp_pthread_cfg_t cfg = esp_pthread_get_default_config();
        cfg.pin_to_core=1;
        esp_pthread_set_cfg(&cfg);
    }

    static String confPath(){
        return "/stairsConf.json";
    }

    bool loadFromFile(){
        fs::File file;
        if(FSStreamInternal::open(file,confPath().c_str(),FSStreamMode::READ)){
            ReadBufferingStream bufferedFile(file, 64); 
            DynamicJsonDocument doc(2048);
            deserializeJson(doc, bufferedFile);
            const JsonObject& root = doc.as<JsonObject>();
            file.close();
            if(deserialize(root)){
                return true;
            }else{
                Serial.println(F("ConnectedStairs::loadFromFile deserialize error"));
                return false;
            }
        }else{
            Serial.println(F("ConnectedStairs::loadFromFile open error"));
            return false;
        }
    }

    bool loadFromMemory(const char* json){
        DynamicJsonDocument doc(2048);
        if(!ArduinoJson::extends::strToJson(json,doc)){
            Serial.println(F("ConnectedStairs::loadFromMemory convertToJSON error"));
            return false;
        }
        const JsonObject& root = doc.as<JsonObject>();
        if(deserialize(root)){
            return true;
        }else{
            Serial.println(F("ConnectedStairs::loadFromMemory deserialize error"));
            return false;
        }
    }

    void setBrightness(unsigned char value){
        _pixelsDriver.setBrightness(value);
    }

    void resetStepxelStatus(){
        for(unsigned int i=0;i< _stepxels.size();i++){
            _stepxels[i].setStatus(false);
        }
    }

    virtual bool deserialize(const JsonObject& root)=0;

    virtual void saveToFile()=0;

    void onLDR(unsigned int value){
        if(value>_LDRThreshold){
            _LDR=true;
        }else{
            _LDR=false;
        }
    }

    virtual bool getStatus() = 0;
    virtual void onLightOff() = 0;
    virtual void onLightOn(const String& color) = 0;
    virtual void onUp2Down() = 0;
    virtual void onDown2Up() = 0;

    virtual void onAnimation() = 0;

};

template<typename T>
class ConnectedStairs : public IConnectedStairs {

        ColorStopMatrix<T> _matrix;
        T _requestColor;

    private:

        void fadeOn(std::function<T(float,float)> func){   
              Serial.println("Task fadeOn running on Core: " + String(xPortGetCoreID()));

            for(uint16_t _fadeLight=0;_fadeLight<256;_fadeLight+=15){
                for(unsigned int y=0; y <_stepxels.size();y++){
                    const Stepxel& stepxel = _stepxels[y];
                    float yy = float(y)/float(_stepxels.size()-1);
                    for(unsigned int x=0;x <stepxel.size();x++ ){
                        float xx = float(x)/float(stepxel.size()-1);
                        {
                            std::unique_lock<std::mutex> lock(_mutex);
                            // RGB requestColor = _matrix.getGradientColor(xx,yy);
                            T requestColor = func(xx,yy);
                            T currrentColor = _pixelsDriver.getPixel(stepxel.begin()+x);
                            T ncolor = FastMath::blend8(currrentColor,requestColor,_fadeLight);
                            _pixelsDriver.setPixel(stepxel.begin()+x,ncolor.pack());
                            _stepxels[y].setStatus(true);
                        }
                    }
                }
                _pixelsDriver.display();
                std::this_thread::sleep_for(std::chrono::milliseconds(_fadems));
            }

            {
                std::unique_lock<std::mutex> lock(_mutex);
                if(_thBreaker==true){
                    _thBreaker=false;
                }
            }

            if(_currentState!=ConnectedStairsState::ON)
                setState(ConnectedStairsState::ON);
        }

        void fadeOff(uint16_t delayms=0){  
              Serial.println("Task fadeOff running on Core: " + String(xPortGetCoreID()));

            if(delayms)std::this_thread::sleep_for(std::chrono::milliseconds(delayms));
            for(uint16_t _fadeLight=0;_fadeLight<256;_fadeLight+=15){
                for(unsigned int y=0; y <_stepxels.size();y++){
                    const Stepxel& stepxel = _stepxels[y];
                    float yy = float(y)/float(_stepxels.size()-1);
                    for(unsigned int x=0;x <stepxel.size();x++ ){
                        float xx = float(x)/float(stepxel.size()-1);
                        {
                            std::unique_lock<std::mutex> lock(_mutex);
                            T currrentColor = _pixelsDriver.getPixel(stepxel.begin()+x);
                            T ncolor = FastMath::blend8by0(currrentColor,_fadeLight);
                            _pixelsDriver.setPixel(stepxel.begin()+x,ncolor.pack());
                        }
                    }
                }
                _pixelsDriver.display();
                std::this_thread::sleep_for(std::chrono::milliseconds(_fadems));
            }
            resetStepxelStatus();
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if(_thBreaker==true){
                    _thBreaker=false;
                }
            }
            
            _isOn = false;

            setState(ConnectedStairsState::OFF);
            Serial.println("lightoff finish");
        }

        void lightDown2Up(){
            bool isBreak=false;
            setState(ConnectedStairsState::DOWN2UP_RUNNING);
            for(uint16_t y=0; y <_stepxels.size();y++){
                const Stepxel& stepxel = _stepxels[y];
                float yy = float(y)/float(_stepxels.size()-1);
                for(uint16_t _fadeLight=0;_fadeLight<256;_fadeLight+=15){
                    for(uint16_t x=0;x <stepxel.size();x++ ){
                        float xx = float(x)/float(stepxel.size()-1);
                        {
                            std::unique_lock<std::mutex> lock(_mutex);
                            T requestColor = _matrix.getGradientColor(xx,yy);
                            T currrentColor = _pixelsDriver.getPixel(stepxel.begin()+x);
                            T ncolor = FastMath::blend8(currrentColor,requestColor,_fadeLight);
                            _pixelsDriver.setPixel(stepxel.begin()+x,ncolor.pack());
                            _stepxels[y].setStatus(true);
                        }
                    }
                    _pixelsDriver.display();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(_lightStepSpeed));
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    if(_thBreaker){
                        y=_stepxels.size();
                        isBreak=true;
                        Serial.println("break thread lightDown2Up");
                    }
                }
            }
            if(!isBreak)
                setState(ConnectedStairsState::DOWN2UP_FINISHED);
        }

        void lightUp2Down(){
            setState(ConnectedStairsState::UP2DOWN_RUNNING);
            bool isBreak=false;
            for(uint16_t y=_stepxels.size(); y>0;y--){
                const Stepxel& stepxel = _stepxels[y];
                float yy = float(y)/float(_stepxels.size()-1);
                for(uint16_t _fadeLight=0;_fadeLight<256;_fadeLight+=15){
                    for(uint16_t x=0;x <stepxel.size();x++ ){
                        float xx = float(x)/float(stepxel.size()-1);
                        {
                            std::unique_lock<std::mutex> lock(_mutex);
                            T requestColor = _matrix.getGradientColor(xx,yy);
                            T currrentColor = _pixelsDriver.getPixel(stepxel.begin()+x);
                            T ncolor = FastMath::blend8(currrentColor,requestColor,_fadeLight);
                            _pixelsDriver.setPixel(stepxel.begin()+x,ncolor.pack());
                            _stepxels[y].setStatus(true);
                        }
                    }
                    _pixelsDriver.display();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(_lightStepSpeed));

                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    if(_thBreaker){
                        y=0;
                        isBreak=true;
                        Serial.println("break thread lightUp2Down");
                    }
                }
                
            }
            if(!isBreak)
                setState(ConnectedStairsState::UP2DOWN_FINISHED);
        }

        void animate(){

            while(false){

            }

        }

    public:

    ConnectedStairs(NeoPixel& pixelsDriver):IConnectedStairs(pixelsDriver){
    }

    void interrupt(){
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _thBreaker=true;
        }
        if(_thLightDown2Up) {
            _thLightDown2Up->join();
            _thLightDown2Up.reset();
        }
        if(_thLightUp2Down) {
            _thLightUp2Down->join();
            _thLightUp2Down.reset();
        }
    }

    bool getStatus(){
      return _isOn;
    }

    bool getConfigurationStatus(){
      return _isConfigure;
    }

    void onUp2Down(){
        Serial.println("onUp2Down");
        if(!_LDR){
            setState(ConnectedStairsState::UP2DOWN_START);
        }
    }

    void onDown2Up(){
        Serial.println("onDown2Up");
        if(!_LDR){
            setState(ConnectedStairsState::DOWN2UP_START);
        }
    }

    void onLightOn(const String& color=T::FullBlack){
        setState(ConnectedStairsState::ON,color);
    }

    void onLightOff(){
        setState(ConnectedStairsState::OFF);
    }

    bool deserialize(const JsonObject& root) {
    
        bool ret = true;
        bool error;
        _stepxels.clear();
        _matrix.clear();

        const JsonArray& stepxelsArray = root["stepxels"];
        uint16_t currentLength = 0;
        for (JsonVariant array : stepxelsArray) {
            _stepxels.emplace_back(array,currentLength);
            if(_stepxels.back().isDirty()) ret = false;
        }
        Serial.print(F("_stepxels size: "));
        Serial.println(currentLength);
        Serial.println(_stepxels.size());

        _pixelsDriver.updateLength(currentLength);
        _pixelsDriver.clear();

        _LDRThreshold = 0;
        error = ArduinoJson::extends::getValueFromJSON<int16_t, uint32_t>(root["LDRThreshold"] | -1 , _LDRThreshold, -1);
        if (!error) {
            Serial.print(F("ConnectedStairs::deserialize LDRThreshold error"));
            ret = false;
            _LDRThreshold = 0;
        }

        _lightOffAfter = 5000;
        error = ArduinoJson::extends::getValueFromJSON<int16_t, uint16_t>(root["lightOffAfter"] | -1 , _lightOffAfter, -1);
        if(!error) {
            Serial.println("ConnectedStairs::deserialize lightOffAfter parsing");
            ret = false;
            _lightOffAfter = 0;
        }

        _lightStepSpeed = 500;
        error = ArduinoJson::extends::getValueFromJSON<int16_t, uint16_t>(root["lightStepSpeed"] | -1 , _lightStepSpeed, -1);
        if(!error) {
            Serial.println("ConnectedStairs::deserialize lightStepSpeed parsing");
            ret = false;
            _lightStepSpeed = 0;
        }

        Serial.println(F("ConnectedStairs : "));
        Serial.print(F("LDRThreshold : "));
        Serial.println(_LDRThreshold);
        Serial.print(F("lightOffAfter : "));
        Serial.println(_lightOffAfter);
        Serial.print(F("lightStepSpeed : "));
        Serial.println(_lightStepSpeed);

        const JsonArray& jsonMatrixGradient = root["matrixGradient"].as<JsonArray>();
        _matrix.configure(jsonMatrixGradient);

        _isConfigure = ret;

        return ret;
    }    

    void saveToFile(){

        Serial.print("saveFile");
        fs::File file;
        if(FSStreamInternal::open(file,confPath().c_str(),FSStreamMode::WRITE)){
            WriteBufferingStream buffered(file, 64);
            DynamicJsonDocument doc(2048);

            doc["LDRThreshold"] = _LDRThreshold;
            doc["lightOffAfter"] = _lightOffAfter;

            JsonArray stepxelsArray = doc.createNestedArray("stepxels");
            for (unsigned int i=0;i< _stepxels.size();i++) {
                JsonObject pix = stepxelsArray.createNestedObject();
                _stepxels[i].serialize(pix);
            }

            JsonArray jsonMatrixGradient = doc.createNestedArray("matrixGradient");
            _matrix.serialize(jsonMatrixGradient);


            serializeJson(doc, Serial);
            serializeJson(doc, buffered);
            buffered.flush();
            file.close();
        }else{
            Serial.print(F("error open Stairs write file"));
        }

    }

    void setState(ConnectedStairsState state,const String& color=T::FullBlack){

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

    void rebootTimer(){
        lightOff();
    }     

    void lightOn(bool isNow=false, const String& color = T::FullBlack){

        _isOn = true;
        _requestColor = color;

        if(isNow){

            interrupt();
            
            if(_thFadeOn) {
                _thFadeOn->join();
                _thFadeOn.reset();
            }

            if(_requestColor.isBlack()){
                _thFadeOn = std::make_shared<std::thread>(&ConnectedStairs<T>::fadeOn, this,[&](float x, float y){                  
                    return _matrix.getGradientColor(x,y);
                });
            }else{
                _thFadeOn = std::make_shared<std::thread>(&ConnectedStairs<T>::fadeOn, this,[&](float x, float y){
                    return _requestColor;
                });
            }

        }else{
            switch(_currentState){
                case ConnectedStairsState::DOWN2UP_START:
                    if(_thLightDown2Up) {
                        _thLightDown2Up->join();
                        _thLightDown2Up.reset();
                    }
                    _thLightDown2Up = std::make_shared<std::thread>(&ConnectedStairs<T>::lightDown2Up, this);
                break;
                case ConnectedStairsState::UP2DOWN_START:
                    if(_thLightUp2Down){
                        _thLightUp2Down->join();
                        _thLightUp2Down.reset();
                    } 
                    _thLightUp2Down = std::make_shared<std::thread>(&ConnectedStairs<T>::lightUp2Down, this);
                break;
            }
        }
    }

    void lightOff(bool isNow = false){

        Serial.println("lightOff");

        if(_thFadeOff){
            _thFadeOff->join();
            _thFadeOff.reset();
        }

        if(isNow){
            _thFadeOff = std::make_shared<std::thread>(&ConnectedStairs<T>::fadeOff, this,0);
        }else{
            _thFadeOff = std::make_shared<std::thread>(&ConnectedStairs<T>::fadeOff, this,_lightOffAfter);
        }
    }

    void onAnimation(){
        _thAnimateLight = std::make_shared<std::thread>(&ConnectedStairs<T>::animate, this);
    }


};

