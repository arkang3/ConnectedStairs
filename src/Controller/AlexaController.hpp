#pragma once

#include "IController.hpp"

#include <map>
#include <thread>
#include <ArduinoJson.h>

#include <fauxmoESP.h>
#include <ESPAsyncWebServer.h> 

#include "../common/Utils.hpp"

class AlexaController : public IController{

        fauxmoESP _fauxmo;
        uint8_t _idDevice;
        
    public:

        enum AlexaColor{
            ALEXA_NONE = 0,
            ALEXA_WARM_WHITE = 383,
            ALEXA_SOFT_WHITE = 350,
            ALEXA_WHITE = 284,
            ALEXA_SUN_WHITE = 234,
            ALEXA_COOL_WHITE = 199,
            ALEXA_RED = 254,
            ALEXA_CRIMSON = 126933,
            ALEXA_SALMON = 6322,
            ALEXA_ORANGE = 14454,
            ALEXA_GOLD = 18458,
            ALEXA_YELLOW = 22100,
            ALEXA_GREEN = 43944,
            ALEXA_TURQUOISE = 63533,
            ALEXA_CYAN = 65790,
            ALEXA_SKY_BLUE = 71831,
            ALEXA_BLUE = 87634,
            ALEXA_PURPLE = 101071,
            ALEXA_MAGENTA = 109480,
            ALEXA_ROSE = 126766,
            ALEXA_LAVENDER = 92969
        };

    private :
        
        std::map<AlexaColor,String> _colorMap;
        std::shared_ptr<std::thread> _looper;

    public:

        AlexaController(const char* name):IController(name){
            _idDevice = 0;
        }

        template<AlexaColor T>
        void setColorFor(String hexa){
            _colorMap[T] = hexa;
        }

        void onGetConnectedStairsConf(FuncBSRefCallback func){}
        void onSetConnectedStairsConf(FuncBSCallback func){}
        void onGetMQTTConf(FuncBSRefCallback func){}
        void onSetMQTTConf(FuncBSCallback func){}
        void onGetNeoPixelConf(FuncBSRefCallback func){}
        void onSetNeoPixelConf(FuncBSCallback func){}

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

                        String aColor = _colorMap[AlexaColor::ALEXA_WHITE];

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

            // Callback to retrieve current state
            _fauxmo.onGetState([&](unsigned char device_id, const char * device_name, bool &state, unsigned char &value) {
                // This is for one device only. You need an array for multiple devices.
                state = _getStatus();
                value = 0;
                if(state)
                    value = _getBrightness();
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

            _looper = std::make_shared<std::thread>(&AlexaController::handle,this);
            _looper->join();
        }

        void handle(){
            _fauxmo.handle();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

       
        

};
