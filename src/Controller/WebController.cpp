#include "WebController.hpp"

#include <ArduinoJson.h>
#include "../common/Utils.hpp"
#include "../fsStream/FSStreamInternal.hpp"

WebController::WebController(const char* name,uint8_t maxRetries):IController(name){
    
   
    

}

void WebController::listen(AsyncWebServer* server){

    server->on("/api.js", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String file;
        FSStreamSD::read(String("/api.js").c_str(),file);
        request->send(200, "text/javascript", file);
    });

    server->on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request){
        String file;
        FSStreamSD::read(String("/w3.css").c_str(),file);
        request->send(200, "text/css", file);
    });

    server->on("/", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String fileContent;
        FSStreamSD::read(String("/index.htm").c_str(),fileContent);
        request->send(200, "text/html", fileContent);
    });
    server->on("/index.html", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String fileContent;
        FSStreamSD::read(String("/index.htm").c_str(),fileContent);
        request->send(200, "text/html", fileContent);
    });
    server->on("/wifi.html", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String file;
        FSStreamSD::read(String("/wifi.htm").c_str(),file);
        request->send(200, "text/html", file);
    });
    server->on("/mqtt.html", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String file;
        FSStreamSD::read(String("/mqtt.htm").c_str(),file);
        request->send(200, "text/html", file);
    });
    server->on("/stairs.html", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String file;
        FSStreamSD::read(String("/stairs.htm").c_str(),file);
        request->send(200, "text/html", file);
    });
    server->on("/neopixel.html", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String file;
        FSStreamSD::read(String("/neopixel.htm").c_str(),file);
        request->send(200, "text/html", file);
    });



    server->on("/getStairsStatus", HTTP_GET, [&](AsyncWebServerRequest * request) {
        String jsonStatus = "{\"status\":\""+String(_getStatus())+",\"bri\":\""+String(_getBrightness())+"\"}";
        request->send(200, "text/html", jsonStatus);
    });

    server->on("/getStairsConfig", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String fileContent;
        if(_funcGetStairsConf(fileContent)){
            request->send(200, "text/html", fileContent);
        }else{
            request->send(200, "text/html", "KO");
        }
    });

    server->on("/setStairsConf", HTTP_GET, [&](AsyncWebServerRequest * request) {
        if(_funcSetStairsConf(request->getParam("JSON")->value()))
            request->send(200, "text/html", "OK");
        else
            request->send(200, "text/html", "KO");

    });

    server->on("/getMQTTConfig", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String fileContent;
        if(_funcGetMQTTConf(fileContent)){
            request->send(200, "text/html", fileContent);
        }else{
            request->send(200, "text/html", "KO");
        }
    });

    server->on("/setMQTTConf", HTTP_GET, [&](AsyncWebServerRequest * request) {
        if(_funcSetMQTTConf(request->getParam("JSON")->value()))
            request->send(200, "text/html", "OK");
        else
            request->send(200, "text/html", "KO");
    });

    server->on("/getNeopixelConfig", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String fileContent;
        if(_funcGetNeoPixelConf(fileContent)){
            request->send(200, "text/html", fileContent);
        }else{
            request->send(200, "text/html", "KO");
        }
    });

    server->on("/setNeopixelConf", HTTP_GET, [&](AsyncWebServerRequest * request) {
        if(_funcSetNeoPixelConf(request->getParam("JSON")->value()))
            request->send(200, "text/html", "OK");
        else
            request->send(200, "text/html", "KO");

    });
   
    server->on("/action", HTTP_GET, [&](AsyncWebServerRequest * request) {

        if (request->hasParam("JSON")){

            String bodyJson = request->getParam("JSON")->value();
            StaticJsonDocument<200> docBody;

            DeserializationError docError = deserializeJson(docBody, bodyJson);
            if (docError) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(docError.f_str());
                request->send(200, "text/plain", "KO");
                return;
            }

            bool error;
            bool on=false;
            error = ArduinoJson::extends::getValueFromJSON<bool, bool>(docBody["on"] | false, on, false);
            if(!on){

                _fnLightOff();
                request->send(200, "text/plain", "OK");
                return;
            }else{

                unsigned char brightness=0;
                error = ArduinoJson::extends::getValueFromJSON<int, unsigned char>(docBody["bri"] | -1 , brightness, -1);
                if(!error){
                    String color;
                    error = ArduinoJson::extends::getValueFromJSON<const char*, String>(docBody["color"] | ((const char*)(NULL)) , color, ((const char*)(NULL)));
                    if(!error){
                        _fnColor("0x00000000");
                        request->send(200, "text/plain", "OK");
                        return;
                    }else{
                        _fnColor(color);
                        request->send(200, "text/plain", "OK");
                        return;
                    }
                }else{

                    _fnBrightness(brightness);
                    request->send(200, "text/plain", "OK");
                }
            }
            request->send(200, "text/plain", "OK");
        }
        
    });

}

void WebController::onLightOff(IController::FuncVVCallback fnLightOff){
    _fnLightOff = fnLightOff;
}

void WebController::onColorChange(IController::FuncVSCallback fnColor){
    _fnColor = fnColor;
}

void WebController::onBrightnessChange(IController::FuncVUCCallback fnBrightness){
    _fnBrightness = fnBrightness;
}

void WebController::onGetConnectedStairsConf(FuncBSRefCallback func) {
    _funcGetStairsConf = func;
}

void WebController::onSetConnectedStairsConf(FuncBSCallback func) {
    _funcSetStairsConf = func;
}

void WebController::onGetMQTTConf(FuncBSRefCallback func) {
    _funcGetMQTTConf = func;
}

void WebController::onSetMQTTConf(FuncBSCallback func) {
    _funcSetMQTTConf = func;
}

void WebController::onGetNeoPixelConf(FuncBSRefCallback func) {
    _funcGetNeoPixelConf = func;
}

void WebController::onSetNeoPixelConf(FuncBSCallback func) {
    _funcSetNeoPixelConf = func;
}
