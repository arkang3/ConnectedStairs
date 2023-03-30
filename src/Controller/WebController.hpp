#pragma once

#include "IController.hpp"
#include <ESPAsyncWebServer.h>

#include "fsStream/FSStreamSD.hpp"


class WebController : public IController{

      


        FuncBSRefCallback _funcGetStairsConf;
        FuncBSCallback _funcSetStairsConf;
        FuncBSRefCallback _funcGetMQTTConf;
        FuncBSCallback _funcSetMQTTConf;
        FuncBSRefCallback _funcGetNeoPixelConf;
        FuncBSCallback _funcSetNeoPixelConf;

    public:

        // void connect(bool restart=false){
        //     WifiConnector::connect(_deviceName,restart);
        // }

        // bool scanNetworks(){
        //     if(WifiConnector::scanNetworks()){
        //         _threadScan.detach();
        //     }
        // }
        
        WebController(const char* name,uint8_t maxRetries=15);

        void listen(AsyncWebServer* server);

        void onLightOff(IController::FuncVVCallback fnLightOff);
        void onColorChange(IController::FuncVSCallback fnColor);
        void onBrightnessChange(IController::FuncVUCCallback fnBrightness);

        void onGetConnectedStairsConf(FuncBSRefCallback func);
        void onSetConnectedStairsConf(FuncBSCallback func);
        void onGetMQTTConf(FuncBSRefCallback func);
        void onSetMQTTConf(FuncBSCallback func);
        void onGetNeoPixelConf(FuncBSRefCallback func);
        void onSetNeoPixelConf(FuncBSCallback func);

};