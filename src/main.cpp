#include <Arduino.h>

// #include "common/BoardPins.hpp"
#include <memory>

#include "FSStream/FSStreamSD.hpp"
#include "FSStream/FSStreamInternal.hpp"

#include "common/Screen.hpp"

#include "driver/Neopixel.hpp"
#include "connectedStairs/ConnectedStairs.hpp"

#include "wifiConnector/wifiConnector.hpp"

#include "Controller/WebController.hpp"
#include "controller/AlexaController.hpp"
#include "controller/MQTTController.hpp"

NeoPixel _pixelsDriver;
std::shared_ptr<IConnectedStairs> stairs;

AsyncWebServer server(80); 
WifiConnector wifi("ConnectedStairs");

WebController web("ConnectedStairs");
AlexaController alexa("ConnectedStairs");
MQTTController mqtt("ConnectedStairs");

bool createStairs();
bool createStairs(String& json);
bool createNeoPixels(String& json);
bool createMQTT(String& json);

void configureAlexaController();
void configureWebController();
void configureMQTTController();

void setup(){

  Serial.begin(115200);
  delay(2000); 

  Serial.println(F("Welcome to ConnectedStairs"));
  Screen::begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, 41, 40);
  Screen::display(0,0,String("Boot"));

  FSStreamInternal::begin();
  FSStreamSD::begin();

  // LittleFS.remove("/stairsConf.json");

  Screen::display(String("."));
  if(FSStreamInternal::exists(NeoPixel::confPath().c_str())){
    if(_pixelsDriver.loadFromFile()){

      switch(_pixelsDriver.getColorModel()){
        case NeoPixel::ColorModel::RGB:
          stairs = std::make_shared< ConnectedStairs<RGB> >(_pixelsDriver);
          break;
        case NeoPixel::ColorModel::RGBW:
          stairs = std::make_shared< ConnectedStairs<RGBW> >(_pixelsDriver);
          break;
        default:
          stairs = std::make_shared< ConnectedStairs<RGB> >(_pixelsDriver);
      }

      if(FSStreamInternal::exists(IConnectedStairs::confPath().c_str())){
        if(stairs->loadFromFile()){
          Serial.println("stairs loaded");
        }else{
          Serial.println("stairsConf error");
        }
      }else{
        Serial.println("stairsConf not found");
      }

    }else{
      Serial.println("neoPixelConf error");
    }
  }else{
    Serial.println("neoPixelConf not found");
  }

  Screen::display(String("."));
  configureWebController();
  configureAlexaController();
  configureMQTTController();
  Screen::display(String(".\n"));

  server.begin();

  Screen::display(0,0,String("Connect to wifi"));
  wifi.autoConnect(server,[](){
    Screen::display(0,0,String("ESP Online"));
    Serial.print("connected on wifi");
    Serial.println("ESP Online");

    web.listen(&server);
    alexa.listen(&server);

    if(FSStreamInternal::exists(MQTTController::confPath().c_str()))
      mqtt.loadFromFile();

  });

}

void loop(){

  std::map< std::pair<uint8_t,uint8_t>, String > multiline;

  String wifiStatus = "Wifi: ";

  if(WiFi.status() == WL_CONNECTED)
    wifiStatus+= WiFi.localIP().toString();
  else
    wifiStatus+="Offline " + WiFi.softAPIP().toString();

  String mqttStatus = "MQTT: ";

  if( mqtt.getStatus())
    mqttStatus+="Online";
  else
    wifiStatus+="Offline";
  
  multiline[std::make_pair(0,0)] = wifiStatus;
  multiline[std::make_pair(0,10)] = mqttStatus;
  multiline[std::make_pair(0,20)] = mqttStatus;


  Screen::display(multiline);
  
  // if(mqtt.getStatus())
  //   Screen::display(String("Online");
  // else
  //   Screen::display(String("Offline");

 

}

void configureWebController(){

  web.setStatus([&](){
    return stairs->getStatus();
  });

  web.setBrightness([&](){
    return _pixelsDriver.getBrightness();
  });

  web.onLightOff([](){
    if(stairs)
      stairs->onLightOff();
  });

  web.onBrightnessChange([](unsigned char value){
      _pixelsDriver.setBrightness(value);
  });

  web.onColorChange([](String color){
    if(stairs)
      stairs->onLightOn(color); 
  });

  web.onGetConnectedStairsConf([&](String& json){
    if(FSStreamInternal::exists(IConnectedStairs::confPath().c_str()))
      if(FSStreamInternal::read(IConnectedStairs::confPath().c_str(),json))
        return true;
    return false;
  }); 

  web.onGetMQTTConf([&](String& json){
    if(FSStreamInternal::exists(MQTTController::confPath().c_str()))
      if(FSStreamInternal::read(MQTTController::confPath().c_str(),json))
        return true;
    return false;
  }); 

  web.onGetNeoPixelConf([&](String& json){
    if(FSStreamInternal::exists(NeoPixel::confPath().c_str()))
      if(FSStreamInternal::read(NeoPixel::confPath().c_str(),json))
        return true;
    return false;
  }); 

  web.onSetConnectedStairsConf([&](String json){
    return createStairs(json);
  }); 

  web.onSetNeoPixelConf([&](String json){
    return createNeoPixels(json);
  });

  web.onSetMQTTConf([&](String json){
   return createMQTT(json);
  });

}

void configureAlexaController(){
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_NONE>("0x000000");

  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_WARM_WHITE>("0xff6200");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_SOFT_WHITE>("0xe9e0c9");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_WHITE>("0xffffff");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_SUN_WHITE>("0xff7300");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_COOL_WHITE>("0xffffff");

  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_RED>("0xff0000");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_CRIMSON>("0xad1c42");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_SALMON>("0xfa8072");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_ORANGE>("0xffa500");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_GOLD>("0xffd700");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_YELLOW>("0xffff00");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_GREEN>("0x00ff00");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_TURQUOISE>("0x40e0d0");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_CYAN>("0x00ffff");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_SKY_BLUE>("0x8abad3");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_BLUE>("0x0000ff");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_PURPLE>("0x800080");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_MAGENTA>("0xff00ff");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_ROSE>("0xff007f");
  alexa.setColorFor<AlexaController::AlexaColor::ALEXA_LAVENDER>("0xe6e6fa");

  alexa.onLightOff([](){
    if(stairs)
      stairs->onLightOff();
  });

  alexa.onBrightnessChange([](unsigned char value){
    _pixelsDriver.setBrightness(value);
  });

  alexa.onColorChange([](String color){
    if(stairs)
      stairs->onLightOn(color); 
  });

  alexa.setStatus([&](){
    if(stairs)
      return stairs->getStatus();
    return false;
  });

  alexa.setBrightness([&](){
    return _pixelsDriver.getBrightness();
  });

}

void configureMQTTController(){

  mqtt.onDisconnected([&](){});

  mqtt.onConnected([](){

    mqtt.on(String("PIRDown"), [](String data){
      bool value = convertToNumeric<String,bool>(data);
      if(value && stairs){
        stairs->onDown2Up();
      }
    });

    mqtt.on(String("PIRUp"),[](String data){
      bool value = convertToNumeric<String,bool>(data);
      if(value && stairs){
        stairs->onUp2Down();
      }
    });

    mqtt.on(String("LDR"),[](String data){
      bool value = convertToNumeric<String,bool>(data);
      if(value && stairs){
        stairs->onDown2Up();
      }
    });

  });
  
}

bool createNeoPixels(String& json){
  if(_pixelsDriver.loadFromMemory(json.c_str())){
      _pixelsDriver.saveToFile();
      createStairs();
      return true;
  }
  Serial.println(F("Can't deserialize mqttConf"));
  return false;
}

bool createStairs(String& json){

  if(stairs)stairs.reset();

  switch(_pixelsDriver.getColorModel()){
    case NeoPixel::ColorModel::RGB:
      stairs = std::make_shared< ConnectedStairs<RGB> >(_pixelsDriver);
      break;
    case NeoPixel::ColorModel::RGBW:
      stairs = std::make_shared< ConnectedStairs<RGBW> >(_pixelsDriver);
      break;
    default:
      stairs = std::make_shared< ConnectedStairs<RGB> >(_pixelsDriver);
  }

  if(FSStreamInternal::exists(IConnectedStairs::confPath().c_str())){
    if(stairs->loadFromMemory(json.c_str())){
      Serial.println("stairs loaded");
      return true;
    }else{
      Serial.println("stairsConf error");
    }
  }else{
    Serial.println("stairsConf not found");
  }
  return false;
}

bool createStairs(){

  if(stairs)stairs.reset();

  switch(_pixelsDriver.getColorModel()){
    case NeoPixel::ColorModel::RGB:
      stairs = std::make_shared< ConnectedStairs<RGB> >(_pixelsDriver);
      break;
    case NeoPixel::ColorModel::RGBW:
      stairs = std::make_shared< ConnectedStairs<RGBW> >(_pixelsDriver);
      break;
    default:
      stairs = std::make_shared< ConnectedStairs<RGB> >(_pixelsDriver);
  }

  if(FSStreamInternal::exists(IConnectedStairs::confPath().c_str())){
    if(stairs->loadFromFile()){
      Serial.println("stairs loaded");
      return true;
    }else{
      Serial.println("stairsConf error");
    }
  }else{
    Serial.println("stairsConf not found");
  }
  return false;
}

bool createMQTT(String& json){
  if(mqtt.loadFromMemory(json.c_str())){
      mqtt.saveToFile();
      return true;
  }
  Serial.println(F("Can't deserialize mqttConf"));
  return false;
}