#pragma once

class IController{

    public:

        String _deviceName;

        typedef std::function<void()> FuncVVCallback;
        typedef std::function<void(String)> FuncVSCallback;
        typedef std::function<void(unsigned char)> FuncVUCCallback;

        typedef std::function<bool(String)> FuncBSCallback;
        typedef std::function<bool(unsigned char)> FuncBUCCallback;
        typedef std::function<bool()> FuncBVCallback;

        FuncVSCallback _fnColor;
        FuncVUCCallback _fnBrightness;
        FuncVVCallback _fnLightOff;

        IController(const char* name){
            _deviceName = name;
        }

        virtual void onLightOff(FuncVVCallback func) = 0;

        virtual void onColorChange(FuncVSCallback func) = 0;

        virtual void onBrightnessChange(FuncVUCCallback func) = 0;

        virtual void onConnectedStairsConf(FuncBSCallback func) = 0;
      
        virtual void onMQTTConf(FuncBSCallback func) = 0;

        virtual void setServiceStatus(String service, FuncBVCallback func) = 0;

};