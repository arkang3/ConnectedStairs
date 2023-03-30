#include "NeoPixel.hpp"

#include <StreamUtils.h>
#include "../fsStream/FSStreamMode.hpp"
#include "../fsStream/FSStreamInternal.hpp"

NeoPixel::NeoPixel(){
    _dataTransmiterMap["NEO_RGB"] = NEO_RGB;
    _dataTransmiterMap["NEO_RBG"] = NEO_RBG;
    _dataTransmiterMap["NEO_GRB"] = NEO_GRB;
    _dataTransmiterMap["NEO_GBR"] = NEO_GBR;
    _dataTransmiterMap["NEO_BRG"] = NEO_BRG;
    _dataTransmiterMap["NEO_BGR"] = NEO_BGR;
    _dataTransmiterMap["NEO_WRGB"] = NEO_WRGB;
    _dataTransmiterMap["NEO_WRBG"] = NEO_WRBG;
    _dataTransmiterMap["NEO_WGRB"] = NEO_WGRB;
    _dataTransmiterMap["NEO_WGBR"] = NEO_WGBR;
    _dataTransmiterMap["NEO_WBRG"] = NEO_WBRG;
    _dataTransmiterMap["NEO_WBGR"] = NEO_WBGR;
    _dataTransmiterMap["NEO_RWGB"] = NEO_RWGB;
    _dataTransmiterMap["NEO_RWBG"] = NEO_RWBG;
    _dataTransmiterMap["NEO_RGWB"] = NEO_RGWB;
    _dataTransmiterMap["NEO_RGBW"] = NEO_RGBW;
    _dataTransmiterMap["NEO_RBWG"] = NEO_RBWG;
    _dataTransmiterMap["NEO_RBGW"] = NEO_RBGW;
    _dataTransmiterMap["NEO_GWRB"] = NEO_GWRB;
    _dataTransmiterMap["NEO_GWBR"] = NEO_GWBR;
    _dataTransmiterMap["NEO_GRWB"] = NEO_GRWB;
    _dataTransmiterMap["NEO_GRBW"] = NEO_GRBW;
    _dataTransmiterMap["NEO_GBWR"] = NEO_GBWR;
    _dataTransmiterMap["NEO_GBRW"] = NEO_GBRW;
    _dataTransmiterMap["NEO_BWRG"] = NEO_BWRG;
    _dataTransmiterMap["NEO_BWGR"] = NEO_BWGR;
    _dataTransmiterMap["NEO_BRWG"] = NEO_BRWG;
    _dataTransmiterMap["NEO_BRGW"] = NEO_BRGW;
    _dataTransmiterMap["NEO_BGWR"] = NEO_BGWR;
    _dataTransmiterMap["NEO_BGRW"] = NEO_BGRW;

    _dataFreq["NEO_KHZ800"] = NEO_KHZ800;
    _dataFreq["NEO_KHZ400"] = NEO_KHZ400;
}

void NeoPixel::convertToType(){

    String dataTransmiter;
    String dataFreq;
    // TODO: check n
    int n = sscanf(_strNeoPixeltype.c_str(), "%s %s", &dataTransmiter, &dataFreq);

    //if()
        auto it = _dataTransmiterMap.find(dataTransmiter);
        if(it != _dataTransmiterMap.end()){
            _neoPixeltype = it->second;
            if(_neoPixeltype == NEO_RGB || _neoPixeltype == NEO_RBG || 
            _neoPixeltype == NEO_GRB || _neoPixeltype == NEO_GBR ||
            _neoPixeltype == NEO_BRG || _neoPixeltype == NEO_BGR){
                Serial.println("rgb!!");
                _colorModel = ColorModel::RGB;
            }else{
                Serial.println("rgbw!!");
                _colorModel = ColorModel::RGBW;
            }
        }else{
            _neoPixeltype = NEO_GRB;
            _colorModel = ColorModel::RGB;
        }

        auto it2 = _dataFreq.find(dataFreq);
        if(it2 != _dataFreq.end()){
            _neoPixeltype += it2->second;
        }else{
            _neoPixeltype += NEO_KHZ800;
        }
    //}
}

NeoPixel::ColorModel NeoPixel::getColorModel(){
    return _colorModel;
}

bool NeoPixel::loadFromFile(){

    fs::File file;
    if(FSStreamInternal::open(file,confPath().c_str(),FSStreamMode::READ)){
        ReadBufferingStream bufferedFile(file, 64); 
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, bufferedFile);
        const JsonObject& root = doc.as<JsonObject>();
        file.close();
        if(deserialize(root)){
            return true;
        }else{
            Serial.println(F("NeoPixel::deserialize error"));
            return false;
        }
    }else{
        Serial.println(F("FSStream::load error"));
        return false;
    }
}

bool NeoPixel::loadFromMemory(const char* json){

    DynamicJsonDocument doc(1024);
    if(!ArduinoJson::extends::strToJson(json,doc))
        return false;

    const JsonObject& root = doc.as<JsonObject>();

    if( deserialize(root)){
        return true;
    }else{
        Serial.println(F("NeoPixel::deserialize error"));
        return false;
    }
}

bool NeoPixel::deserialize(const JsonObject& object){

    bool error = true;     
    _pin = 8;
    error = ArduinoJson::extends::getValueFromJSON<int16_t, uint16_t>(object["pin"] | -1, _pin, -1 );
    if (!error) {
        Serial.println(F("NeoPixel::pin error parsing"));
        return false;
    }
    _brightness = 35;
    error = ArduinoJson::extends::getValueFromJSON<int16_t, uint8_t>(object["brightness"] | -1 , _brightness, -1);
    if (!error) {
        Serial.println(F("NeoPixel::brightness error parsing"));
        return false;
    }
    _maxBrightness = 35;
    error = ArduinoJson::extends::getValueFromJSON<int16_t, uint8_t>(object["maxBrightness"] | -1 , _maxBrightness, -1);
    if (!error) {
        Serial.println(F("NeoPixel::maxBrightness error parsing"));
        return false;
    }
    _strNeoPixeltype = "NEO_GRB NEO_KHZ800";
    error = ArduinoJson::extends::getValueFromJSON<const char*, String>(object["type"] | ((const char*)(NULL)) , _strNeoPixeltype, ((const char*)(NULL)));
    if (!error) {
        Serial.println(F("NeoPixel::type error parsing"));
        return false;
    }

    if(_brightness>_maxBrightness){
        _brightness = _maxBrightness;
    }

    // _pin=17;
    Serial.println(F("Neopixel: "));
    Serial.print(F("    _pin: "));
    Serial.println(_pin);
    Serial.print(F("    _type: "));
    Serial.println(_strNeoPixeltype);
    Serial.print(F("    _brightness: "));
    Serial.println(_brightness);
    Serial.print(F("    _maxBrightness: "));
    Serial.println(_maxBrightness);

    convertToType();

    //TODO: NEO_GRB  + NEO_KHZ800
    _neopixels = std::make_shared<Adafruit_NeoPixel>(0,_pin,NEO_GRB + NEO_KHZ800);
    _neopixels->begin();
    _neopixels->clear();
    _neopixels->setBrightness(_brightness);
    _neopixels->show();

    return true;
}

void NeoPixel::updateLength(uint16_t size){
    _size = size;
    _neopixels->updateLength(_size);
}

void NeoPixel::saveToFile(){
            
    fs::File file;
    if(FSStreamInternal::open(file,confPath().c_str(),FSStreamMode::WRITE)){
        WriteBufferingStream buffered(file, 64);
        DynamicJsonDocument doc(1024);

        doc["pin"]= _pin;
        doc["brightness"]= _brightness;
        doc["maxBrightness"]= _maxBrightness;
        doc["type"]= _strNeoPixeltype;

        serializeJson(doc, buffered);
        buffered.flush();
    }else{
        Serial.print(F("error open NeoPixel write file"));
    }
    file.close();
}

uint8_t NeoPixel::getBrightnessMax(){
    return _maxBrightness;
}

uint8_t NeoPixel::getBrightness(){
    return _brightness;
}

void NeoPixel::setBrightness(const uint8_t& value){
    if(_brightness!=value && value <=_maxBrightness){
        std::lock_guard<std::mutex> guard(_driverMutex);
        _neopixels->setBrightness(value);
        _neopixels->show();
        _brightness = value;
    }
}

void NeoPixel::clear() {
    std::lock_guard<std::mutex> guard(_driverMutex);
    _neopixels->clear();
    _neopixels->show();
}

void NeoPixel::display(){
    std::lock_guard<std::mutex> guard(_driverMutex);
    _neopixels->show();
}

void NeoPixel::display(const uint32_t& color){
    std::lock_guard<std::mutex> guard(_driverMutex);
    _neopixels->fill(color,0);
    _neopixels->show();
}

void NeoPixel::display(const uint16_t& begin, const uint16_t& size, const uint32_t& color){
    std::lock_guard<std::mutex> guard(_driverMutex);
    _neopixels->fill(color,begin,size);
    _neopixels->show();
}

uint32_t NeoPixel::getPixel(const uint16_t& position){
    return _neopixels->getPixelColor(position);
}

void NeoPixel::setPixel(const uint16_t& position, const uint32_t& color){
    std::lock_guard<std::mutex> guard(_driverMutex);
    _neopixels->setPixelColor(position,color);
}

void NeoPixel::setPixel(const uint16_t& position, const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& w){
    std::lock_guard<std::mutex> guard(_driverMutex);
    _neopixels->setPixelColor(position,r,g,b,w);
}

void NeoPixel::setPixel(const uint16_t&position, const uint8_t& r, const uint8_t& g, const uint8_t& b){
    std::lock_guard<std::mutex> guard(_driverMutex);
    _neopixels->setPixelColor(position,r,g,b);
}

