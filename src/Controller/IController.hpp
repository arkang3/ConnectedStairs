#pragma once

#include <Arduino.h>
#include <functional>

class IController{

    public:

        String _deviceName;

        typedef std::function<void()> FuncVVCallback;
        typedef std::function<void(String)> FuncVSCallback;
        typedef std::function<void(unsigned char)> FuncVUCCallback;

        typedef std::function<bool(String)> FuncBSCallback;
        typedef std::function<bool(String&)> FuncBSRefCallback;
        typedef std::function<bool(unsigned char)> FuncBUCCallback;
        typedef std::function<bool()> FuncBVCallback;

        typedef std::function<unsigned int()> FuncUIVCallback;

        FuncVSCallback _fnColor;
        FuncVUCCallback _fnBrightness;
        FuncVVCallback _fnLightOff;

        FuncBVCallback _getStatus;
        FuncUIVCallback _getBrightness;

        IController(const char* name){
            _deviceName = name;
        }

        virtual void onLightOff(FuncVVCallback func) = 0;
        virtual void onColorChange(FuncVSCallback func) = 0;
        virtual void onBrightnessChange(FuncVUCCallback func) = 0;

        virtual void onGetConnectedStairsConf(FuncBSRefCallback func) = 0;
        virtual void onSetConnectedStairsConf(FuncBSCallback func) = 0;
        virtual void onGetMQTTConf(FuncBSRefCallback func) = 0;
        virtual void onSetMQTTConf(FuncBSCallback func) = 0;
        virtual void onGetNeoPixelConf(FuncBSRefCallback func) = 0;
        virtual void onSetNeoPixelConf(FuncBSCallback func) = 0;

        void setStatus(FuncBVCallback func){
            _getStatus = func;
        }

        void setBrightness(FuncUIVCallback func){
            _getBrightness = func;
        }

};