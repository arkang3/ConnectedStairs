#pragma once

#include "IController.hpp"
#include <map>

#include <fauxmoESP.h>
#include <ESPAsyncWebServer.h> 
#include <Ticker.h>
#include <ArduinoJson.h>

#include "../Common/Utils.hpp"
// class AlexaController : public IController {

    	// blanc chaud 383
			// blanc doux 350
			// blanc 284
			// blanc luliere du jour 234
			// blanc froid 199

			

			// rouge h 0 s 254
			// cramoisi h 63351 s 231
			// saumon h 3095 s 132
			// orange h 7100 s 254
			// or 9102 h 254
			// jaune 10923 s 254
			// vert h 21845 s 254
			// turquoise h 31675 s 183
			// cyan h 32768 s 254
			// bleu ciel h 35862 s 107
			// bleu 43690 s 254
			// violet 50426 s 219
			// magenta h 54613 s 254
			// rose h 63351 s 64
			// lavande h 46421 s 127

class AlexaController : public IController{

        fauxmoESP _fauxmo;
        unsigned char _idDevice;
        std::map<String,FuncBVCallback> _serviceMap;

    public:

        enum AlexaColor{
            NONE = 0,
            WARM_WHITE = 383,
            SOFT_WHITE = 350,
            WHITE = 284,
            SUN_WHITE = 234,
            COOL_WHITE = 199,
            RED = 254,
            CRIMSON = 126933,
            SALMON = 6322,
            ORANGE = 14454,
            GOLD = 18458,
            YELLOW = 22100,
            GREEN = 43944,
            TURQUOISE = 63533,
            CYAN = 65790,
            SKY_BLUE = 71831,
            BLUE = 87634,
            PURPLE = 101071,
            MAGENTA = 109480,
            ROSE = 126766,
            LAVENDER = 92969
        };

    private :
        
        std::map<AlexaColor,String> _colorMap;
        Ticker _looper;

    public:

        AlexaController(const char* name):IController(name){
          
            // colorMap[0] = AlexaColor::NONE;

            // colorMap[383] = AlexaColor::WARM_WHITE;
            // colorMap[350] = AlexaColor::SOFT_WHITE;
            // colorMap[284] = AlexaColor::WHITE;
            // colorMap[234] = AlexaColor::SUN_WHITE;
            // colorMap[199] = AlexaColor::COOL_WHITE;

            // colorMap[254] = AlexaColor::RED;
            // colorMap[126933] = AlexaColor::CRIMSON;
            // colorMap[6322] = AlexaColor::SALMON;
            // colorMap[14454] = AlexaColor::ORANGE;
            // colorMap[18458] = AlexaColor::GOLD;
            // colorMap[22100] = AlexaColor::YELLOW;
            // colorMap[43944] = AlexaColor::GREEN;
            // colorMap[63533] = AlexaColor::TURQUOISE;
            // colorMap[65790] = AlexaColor::CYAN;
            // colorMap[71831] = AlexaColor::SKY_BLUE;
            // colorMap[87634] = AlexaColor::BLUE;
            // colorMap[101071] = AlexaColor::PURPLE;
            // colorMap[109480] = AlexaColor::MAGENTA;
            // colorMap[126766] = AlexaColor::ROSE;
            // colorMap[92969] = AlexaColor::LAVENDER;
        
        }

        template<AlexaColor T>
        void setColorFor(String hexa){
                _colorMap[T] = hexa;
        }

        
        void onConnectedStairsConf(FuncBSCallback func){
        };
      
        void onMQTTConf(FuncBSCallback func){
        };

        void onLightOff(IController::FuncVVCallback fnLightOff){
            _fnLightOff = fnLightOff;
        }

        void onColorChange(IController::FuncVSCallback fnColor){
            _fnColor = fnColor;
        }

        void onBrightnessChange(IController::FuncVUCCallback fnBrightness){
            _fnBrightness = fnBrightness;
        }

        void listen(AsyncWebServer* server){
            _fauxmo.createServer(false);
            _fauxmo.setPort(80);
            _fauxmo.enable(true);
            _idDevice = _fauxmo.addDevice(_deviceName.c_str());

            _fauxmo.onSetState([this](unsigned char device_id, const char* device_name,String urlJson, String bodyJson){
                
                StaticJsonDocument<200> docBody;

                DeserializationError docError = deserializeJson(docBody, bodyJson);
                if (docError) {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(docError.f_str());
                    return;
                }
                //serializeJson(docBody, Serial);

                bool error;

                bool on=false;
                error = ArduinoJson::extends::getValueFromJSON<bool, bool>(docBody["on"] | false, on, false);
                if(!on){

                    _fnLightOff();
                    return;

                }else{

                    unsigned int value=0;
                    unsigned char brightness=0;
                    error = ArduinoJson::extends::getValueFromJSON<int, unsigned char>(docBody["bri"] | -1 , brightness, -1);
                    if(!error){
                        unsigned int ct;
                        error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(docBody["ct"] | -1 , ct, -1);
                        if(!error){
                            unsigned int hue=0;
                            error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(docBody["hue"] | -1 , hue, -1);
                            unsigned int sat=0;
                            error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(docBody["sat"] | -1 , sat, -1);
                            value = hue*2 + sat;
                            if(!error){
                                String aColor = "0x000000";
                                _fnColor(aColor);
                                return;
                            }
                        }else{
                            value = ct;
                        }

                        String aColor = _colorMap[AlexaColor::WHITE];

                        std::map<AlexaColor,String>::iterator it = _colorMap.find(AlexaColor(value));
                        if (it != _colorMap.end()){
                            aColor = it->second;
                        }
                        
                        _fnColor(aColor);                       
                        
                    }else{

                        _fnBrightness(brightness);
                        
                    }
                }
                
            });

            // These two callbacks are required for gen1 and gen3 compatibility
            server->onRequestBody([this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
                if (_fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), String((char *)data))) return;
                // Handle any other body request here...
            });
            server->onNotFound([this](AsyncWebServerRequest * request) {
                String body = (request->hasParam("body", true)) ? request->getParam("body", true)->value() : String();
                if (_fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), body)) return;
                // Handle not found request here...
            }); 

            _looper.attach_ms(200,[&](){
                if(_serviceMap["getStairsStatus"]())
                    _fauxmo.setState(_idDevice, true, 255);
                else
                    _fauxmo.setState(_idDevice, false, 0);
                _fauxmo.handle();
            });
        }

        void setServiceStatus(String service, FuncBVCallback func){
            _serviceMap[service] = func;
        }
        

};
