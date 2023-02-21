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

    Serial.print(F("LDRThreshold : "));
    Serial.println(_LDRThreshold);

    const JsonObject& jsonAnimator = root["lightEffect"].as<JsonObject>();
    _animator = animatorBuilder(jsonAnimator);
    if(_animator->isDirty()) ret = false;

    _isConfigure = ret;

    return ret;
}

 void ConnectedStairs::saveToFile(String confPath){

    Serial.print("saveFile");
    File file;
    if(FSStream::open(confPath,file,"w")){
        WriteBufferingStream buffered(file, 64);
        DynamicJsonDocument doc(2048);

        doc["LDRThreshold"]= _LDRThreshold;

        JsonObject jsonNeoPixel = doc.createNestedObject("neopixel");
        _pixelsDriver.serialize(jsonNeoPixel);

        JsonArray stepxelsArray = doc.createNestedArray("stepxels");
        for (unsigned int i=0;i< _stepxels.size();i++) {
            JsonObject pix = stepxelsArray.createNestedObject();
            _stepxels[i].serialize(pix);
        }

        JsonObject jsonAnimator = doc.createNestedObject("lightEffect");
        _animator->serialize(jsonAnimator);

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
        _animator->setState(ConnectedStairsState::DOWN2UP_START);
    }
}

void ConnectedStairs::onUp2Down(){
    Serial.println("onUp2Down");
    if(!_LDR){
        _animator->setState(ConnectedStairsState::UP2DOWN_START);
    }
}

void ConnectedStairs::lightOn(const RGBW& color){
    _animator->setState(ConnectedStairsState::ON,color);
}

void ConnectedStairs::lightOff(){
    _animator->setState(ConnectedStairsState::OFF);
}
