#include "ConnectedStairs.hpp"
#include <StreamUtils.h>

#include "FSStream.hpp"
#include "Utils.hpp"

bool ConnectedStairs::loadFromFile(String confPath){

    File file;
    if(FSStream::open(confPath,file,"r")){
        ReadBufferingStream bufferedFile(file, 64); 
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, bufferedFile);
        const JsonObject& root = doc.as<JsonObject>();
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

bool ConnectedStairs::loadFromMemory(const char* json){
   
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

bool ConnectedStairs::deserialize(const JsonObject& root) {
    
    bool ret = true;
    bool error;
    _stepxels.clear();
    _matrix.clear();

    const JsonArray& stepxelsArray = root["stepxels"];
    unsigned int currentLength = 0;
    for (JsonVariant array : stepxelsArray) {
        _stepxels.emplace_back(array,currentLength);
        if(_stepxels.back().isDirty()) ret = false;
    }
    Serial.print(F("_stepxels size: "));
    Serial.println(_stepxels.size());

    const JsonObject& jsonNeoPixel = root["neopixel"].as<JsonObject>();
    _pixelsDriver.configure(jsonNeoPixel,currentLength);
    _pixelsDriver.clear();
    if(_pixelsDriver.isDirty()) ret = false;

    _LDRThreshold = 0;
    error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(root["LDRThreshold"] | -1 , _LDRThreshold, -1);
    if (!error) {
        Serial.print(F("ConnectedStairs::deserialize LDRThreshold error"));
        ret = false;
        _LDRThreshold = 0;
    }

    _lightOffAfter = 5000;
    error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(root["lightOffAfter"] | -1 , _lightOffAfter, -1);
    if(!error) {
        Serial.println("ConnectedStairs::deserialize lightOffAfter parsing");
        ret = false;
        _lightOffAfter = 0;
    }

    Serial.print(F("LDRThreshold : "));
    Serial.println(_LDRThreshold);

    const JsonArray& jsonMatrixGradient = root["matrixGradient"].as<JsonArray>();
    _matrix.configure(jsonMatrixGradient);

    _lightEffect = lightEffectBuilder(root["lightEffect"]);
    
    _isConfigure = ret;

    return ret;
}

 void ConnectedStairs::saveToFile(String confPath){

    Serial.print("saveFile");
    File file;
    if(FSStream::open(confPath,file,"w")){
        WriteBufferingStream buffered(file, 64);
        DynamicJsonDocument doc(2048);

        doc["LDRThreshold"] = _LDRThreshold;
        doc["lightOffAfter"] = _lightOffAfter;

        JsonObject jsonNeoPixel = doc.createNestedObject("neopixel");
        _pixelsDriver.serialize(jsonNeoPixel);

        JsonArray stepxelsArray = doc.createNestedArray("stepxels");
        for (unsigned int i=0;i< _stepxels.size();i++) {
            JsonObject pix = stepxelsArray.createNestedObject();
            _stepxels[i].serialize(pix);
        }

        JsonArray jsonMatrixGradient = doc.createNestedArray("matrixGradient");
        _matrix.serialize(jsonMatrixGradient);


        JsonObject jsonLightEffect = doc.createNestedObject("lightEffect");
        _lightEffect->serialize(jsonLightEffect);

        serializeJson(doc, Serial);
        serializeJson(doc, buffered);
        buffered.flush();
        file.close();
    }else{
        Serial.print(F("error open Stairs write file"));
    }

 }

 void ConnectedStairs::onLDR(unsigned int value){
    if(value>_LDRThreshold){
        _LDR=true;
    }else{
        _LDR=false;
    }
}

void ConnectedStairs::onDown2Up(){
    Serial.println("onDown2Up");
    if(!_LDR){
        setState(ConnectedStairsState::DOWN2UP_START);
    }
}

void ConnectedStairs::onUp2Down(){
    Serial.println("onUp2Down");
    if(!_LDR){
        setState(ConnectedStairsState::UP2DOWN_START);
    }
}

void ConnectedStairs::onlightOn(const RGBW& color){
    setState(ConnectedStairsState::ON,color);
}

void ConnectedStairs::onlightOff(){
    setState(ConnectedStairsState::OFF);
}

void ConnectedStairs::setState(ConnectedStairsState state,const RGBW& color){

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

void ConnectedStairs::rebootTimer(){
    _resetTimer = millis();
    lightOff();
}     

void ConnectedStairs::lightOn(bool isNow, const RGBW& color){

    _isOn = true;

    if(isNow){

        _lightEffect->interrupt();
        
        if(color.isBlack()){

            _fadeLight = 1;
            _threadFade.attach_ms(_fadems,[&](){

                if(_fadeLight>0){
                    for(unsigned int y=0; y <_stepxels.size();y++){

                        const Stepxel& stepxels = _stepxels[y];
                        std::function<RGBW(unsigned int, RGBW, float)> colorFunc = [&](unsigned int x,RGBW currrentColor, float brightness){
                            float yy = float(y)/(_stepxels.size()-1);
                            float xx = float(x)/(_stepxels[y].size()-1);
                            RGBW requestColor = _matrix.getGradientColor(xx,yy);
                            unsigned char r = currrentColor.getRedColor<int>()*brightness + requestColor.getRedColor<int>()*(1-brightness);
                            unsigned char g = currrentColor.getGreenColor<int>()*brightness + requestColor.getGreenColor<int>()*(1-brightness);
                            unsigned char b = currrentColor.getBlueColor<int>()*brightness + requestColor.getBlueColor<int>()*(1-brightness);
                            unsigned char w = currrentColor.getWhiteColor<int>()*brightness + requestColor.getWhiteColor<int>()*(1-brightness);
                            RGBW color(r,g,b,w);
                            return color;
                        };
                        
                        _pixelsDriver.setColorLine(stepxels.begin(),stepxels.size(),colorFunc,_fadeLight);
                        const_cast<Stepxel&>(stepxels).setStatus(true);                            
                    }
                    _pixelsDriver.display();
                    _fadeLight-=0.05f;
                }else{
                    _threadFade.detach();
                }
            });

        }else{
            _fadeLight = 1;
            _requestColor = color;
            _threadFade.attach_ms(_fadems,[&](){

                if(_fadeLight>0){
                    for(unsigned int y=0; y <_stepxels.size();y++){

                        const Stepxel& stepxels = _stepxels[y];
                        std::function<RGBW(RGBW, float)> colorFunc = [&](RGBW currrentColor, float brightness){
                            unsigned char r = currrentColor.getRedColor<int>()*brightness + _requestColor.getRedColor<int>()*(1-brightness);
                            unsigned char g = currrentColor.getGreenColor<int>()*brightness + _requestColor.getGreenColor<int>()*(1-brightness);
                            unsigned char b = currrentColor.getBlueColor<int>()*brightness + _requestColor.getBlueColor<int>()*(1-brightness);
                            unsigned char w = currrentColor.getWhiteColor<int>()*brightness + _requestColor.getWhiteColor<int>()*(1-brightness);
                            RGBW color(r,g,b,w);
                            return color;
                        };
                        
                        _pixelsDriver.setColorLine(stepxels.begin(),stepxels.size(),colorFunc,_fadeLight);
                        const_cast<Stepxel&>(stepxels).setStatus(true);                            
                    }
                    _pixelsDriver.display();
                    _fadeLight-=0.05f;
                }else{
                    _threadFade.detach();
                }
            });
        }
        if(_currentState!=ConnectedStairsState::ON)
            setState(ConnectedStairsState::ON);

    }else{
        _resetTimer= _startTimer = millis();
        switch(_currentState){
            case ConnectedStairsState::DOWN2UP_START:
                _lightEffect->lightDown2Up(_ptrSetState);
            break;
            case ConnectedStairsState::UP2DOWN_START:
                _lightEffect->lightUp2Down(_ptrSetState);
            break;

        }
        
    }

}

void ConnectedStairs::lightOff(bool isNow){
    Serial.println("lightOff");
    _isOn = false;
    if(isNow){

        _fadeLight = 1;
        _threadFade.attach_ms(_fadems,[&](){

            if(_fadeLight>=0){
                for(unsigned int y=0; y <_stepxels.size();y++){

                    const Stepxel& stepxels = _stepxels[y];
                    std::function<RGBW(RGBW, float)> colorFunc = [&](RGBW currrentColor, float brightness){
                        unsigned char r = currrentColor.getRedColor<int>()*brightness;
                        unsigned char g = currrentColor.getGreenColor<int>()*brightness;
                        unsigned char b = currrentColor.getBlueColor<int>()*brightness;
                        unsigned char w = currrentColor.getWhiteColor<int>()*brightness;
                        RGBW color(r,g,b,w);
                        return color;
                    };
                    
                    _pixelsDriver.setColorLine(stepxels.begin(),stepxels.size(),colorFunc,_fadeLight);
                    const_cast<Stepxel&>(stepxels).setStatus(true);                            
                }
                _pixelsDriver.display();
                _fadeLight-=0.05f;
            }else{

                _threadFade.detach();
                resetStepxelStatus();
                setState(ConnectedStairsState::OFF);
                Serial.println("lightoff finish");
            }
        });

        
    }else{
        // Serial.println("lightOff in " + (_lightOffAfter+_resetTimer-_startTimer));
        _threadOff.detach();
        _threadOff.once_ms(_lightOffAfter,[&](){

            _fadeLight = 1;
            _threadFade.attach_ms(_fadems,[&](){

                if(_fadeLight>=0){
                    for(unsigned int y=0; y <_stepxels.size();y++){

                        const Stepxel& stepxels = _stepxels[y];
                        std::function<RGBW(RGBW, float)> colorFunc = [&](RGBW currrentColor, float brightness){
                            unsigned char r = currrentColor.getRedColor<int>()*brightness;
                            unsigned char g = currrentColor.getGreenColor<int>()*brightness;
                            unsigned char b = currrentColor.getBlueColor<int>()*brightness;
                            unsigned char w = currrentColor.getWhiteColor<int>()*brightness;
                            RGBW color(r,g,b,w);
                            return color;
                        };
                        
                        _pixelsDriver.setColorLine(stepxels.begin(),stepxels.size(),colorFunc,_fadeLight);
                        const_cast<Stepxel&>(stepxels).setStatus(true);                            
                    }
                    _pixelsDriver.display();
                    _fadeLight-=0.05f;
                }else{
                    _threadFade.detach();
                    resetStepxelStatus();
                    setState(ConnectedStairsState::OFF);
                    Serial.println("lightoff finish");
                }
            });        
        });
    }
}

void ConnectedStairs::resetStepxelStatus(){
    for(unsigned int i=0;i< _stepxels.size();i++){
        _stepxels[i].setStatus(false);
    }
}