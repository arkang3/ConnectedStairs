#include <Arduino.h>

#include <ESPAsyncWifiManagerCustom.hpp>
#include <ESP8266mDNS.h>

#include "Common/FSStream.hpp"
#include "Common/ConnectedStairs.hpp"

#include "MQTT/MQTTClient.hpp"

#include "Controller/AlexaController.hpp"
#include "Controller/WebController.hpp"

ConnectedStairs stairs;

AsyncWebServer server(80);
MQTTClient mqttClient;

AlexaController alexa("ConnectedStairs");
WebController web("ConnectedStairs");

std::function<void(String)> pirDownfunc;
std::function<void(String)> pirUpfunc;
std::function<void(String)> LDRFunc;
std::function<void()> mqttConnected;

void configureAlexaController();
void configureWebController();

void configureMQTTClient();


//30LEDs/m
// 5  = 150
// 18 = 540


// 130,00
// 80,00
// 80,00
// 82,00
// 84,00
// 89,50
// 97,00
// 113,00
// 95,00
// 85,50
// 87,50
// 104,00
// 108,00
// 97,00
// 91,00
// 85,00
// 81,50
// 78,50
// 78,50

// 1747,00

void setup(){

  Serial.begin(115200);
  delay(50);
  Serial.print(F("Welcome to ConnectedStairs"));
  Serial.println(ESP.getFreeHeap(),DEC);

  FSStream::begin();

  configureMQTTClient();

  configureWebController();
  web.listen(&server);

  server.begin();

  stairs.loadFromFile();

  web.autoConnect();

  // pirDownfunc =  [&pirDownfunc,&stairs](String data){
  //     bool value = convertToNumeric<String,bool>(data);
  //     if(value){
  //       stairs.onDown2Up();
  //     }
  // };

  // pirUpfunc =  [&pirUpfunc,&stairs](String data){
  //     bool value = convertToNumeric<String,bool>(data);
  //     if(value){
  //       stairs.onUp2Down();
  //     }
  // };

  // LDRFunc =  [&LDRFunc,&stairs](String data){
  //     bool value = convertToNumeric<String,bool>(data);
  //     if(value){
  //       stairs.onDown2Up();
  //     }
  // };


  // mqttConnected=[&mqttConnected,&mqttClient](){
  //   mqttClient.on(String("PIRDown"),pirDownfunc);
  //   mqttClient.on(String("PIRUp"),pirUpfunc);
  //   mqttClient.on(String("LDR"),LDRFunc);
  // };

  // mqttClient.onConnected(mqttConnected);
  // mqttClient.onDisconnected([&](){});


  // if(!web.autoConnect()) {
  //   Serial.println(F("failed to connect and hit timeout"));
  //   delay(3000);
  //   //reset and try again, or maybe put it to deep sleep
  //   ESP.reset();
  //   delay(5000);
  // }

  // configureAlexaController();
  // alexa.listen(&server);


  // // DNSServer dns;
  // // AsyncWiFiManagerCustom wifiManager(&server,&dns);
  // // wifiManager.resetSettings();
  
  // // if (!wifiManager.autoConnect("AP-CONNNECTED-STAIRS")) {
  // //   Serial.println(F("failed to connect and hit timeout"));
  // //   delay(3000);
  // //   //reset and try again, or maybe put it to deep sleep
  // //   ESP.reset();
  // //   delay(5000);
  // // }

}

void loop(){
  // MDNS.update();
  //alexa.handle();
}

void configureWebController(){

  web.onConnected([&](){
    Serial.println("ESP Online");
    if(MDNS.begin("connectedstairs")) {
      MDNS.addService("http", "tcp", 80);
      Serial.println(F("MDNS responder started"));
    }

    configureAlexaController();
    alexa.listen(&server);

    mqttClient.loadFromFile();
  });

  web.setServiceStatus("getStairsStatus",[&](){
    return stairs.getStatus();
  });

  web.setServiceStatus("getMQTTStatus",[&](){
    return mqttClient.getStatus();
  });

  web.onLightOff([](){
    stairs.lightOff();
  });

  web.onBrightnessChange([](unsigned char value){
    Serial.println(value);
    stairs.setBrightness(value);
  });

  web.onColorChange([](String acolor){
    Serial.println("web::onColorChange");
    RGBW color(acolor);
    stairs.lightOn(color); 
  });

  web.onConnectedStairsConf([&](String json){
    if(stairs.loadFromMemory(json.c_str())){
      stairs.saveToFile();
      return true;
    }else{
      Serial.println(F("Can't deserialize StairsConf"));
      return false;
    }    
  });

  web.onMQTTConf([&](String json){
    if(mqttClient.loadFromMemory(json.c_str())){
      mqttClient.connect();
      mqttClient.saveToFile();
    }else{
      Serial.println(F("Can't deserialize mqttConf"));
      return false;
    }
    return true;
  });

}

void configureAlexaController(){
  alexa.setColorFor<AlexaController::AlexaColor::NONE>("0x000000");

  alexa.setColorFor<AlexaController::AlexaColor::WARM_WHITE>("0xff6200");
  alexa.setColorFor<AlexaController::AlexaColor::SOFT_WHITE>("0xe9e0c9");
  alexa.setColorFor<AlexaController::AlexaColor::WHITE>("0xffffff");
  alexa.setColorFor<AlexaController::AlexaColor::SUN_WHITE>("0xff7300");
  alexa.setColorFor<AlexaController::AlexaColor::COOL_WHITE>("0xffffff");

  alexa.setColorFor<AlexaController::AlexaColor::RED>("0xff0000");
  alexa.setColorFor<AlexaController::AlexaColor::CRIMSON>("0xad1c42");
  alexa.setColorFor<AlexaController::AlexaColor::SALMON>("0xfa8072");
  alexa.setColorFor<AlexaController::AlexaColor::ORANGE>("0xffa500");
  alexa.setColorFor<AlexaController::AlexaColor::GOLD>("0xffd700");
  alexa.setColorFor<AlexaController::AlexaColor::YELLOW>("0xffff00");
  alexa.setColorFor<AlexaController::AlexaColor::GREEN>("0x00ff00");
  alexa.setColorFor<AlexaController::AlexaColor::TURQUOISE>("0x40e0d0");
  alexa.setColorFor<AlexaController::AlexaColor::CYAN>("0x00ffff");
  alexa.setColorFor<AlexaController::AlexaColor::SKY_BLUE>("0x8abad3");
  alexa.setColorFor<AlexaController::AlexaColor::BLUE>("0x0000ff");
  alexa.setColorFor<AlexaController::AlexaColor::PURPLE>("0x800080");
  alexa.setColorFor<AlexaController::AlexaColor::MAGENTA>("0xff00ff");
  alexa.setColorFor<AlexaController::AlexaColor::ROSE>("0xff007f");
  alexa.setColorFor<AlexaController::AlexaColor::LAVENDER>("0xe6e6fa");

  alexa.onLightOff([](){
    stairs.lightOff();
  });

  alexa.onBrightnessChange([](unsigned char value){
    Serial.println(value);
    stairs.setBrightness(value);
  });

  alexa.onColorChange([](String acolor){
    Serial.print("alexa::onColorChange");
    RGBW color(acolor);
    stairs.lightOn(color); 
  });

  alexa.setServiceStatus("getStairsStatus",[&](){
    return stairs.getStatus();
  });
}


void configureMQTTClient(){

  mqttClient.onDisconnected([&](){});

// pirDownfunc =  [&pirDownfunc,&stairs](String data){
  //     bool value = convertToNumeric<String,bool>(data);
  //     if(value){
  //       stairs.onDown2Up();
  //     }
  // };

  // pirUpfunc =  [&pirUpfunc,&stairs](String data){
  //     bool value = convertToNumeric<String,bool>(data);
  //     if(value){
  //       stairs.onUp2Down();
  //     }
  // };

  // LDRFunc =  [&LDRFunc,&stairs](String data){
  //     bool value = convertToNumeric<String,bool>(data);
  //     if(value){
  //       stairs.onDown2Up();
  //     }
  // };

  mqttClient.onConnected([&stairs](){

    mqttClient.on(String("PIRDown"), [&stairs](String data){
      bool value = convertToNumeric<String,bool>(data);
      if(value){
        stairs.onDown2Up();
      }
    });

    mqttClient.on(String("PIRUp"),[&stairs](String data){
      bool value = convertToNumeric<String,bool>(data);
      if(value){
        stairs.onUp2Down();
      }
    });
    mqttClient.on(String("LDR"),[&stairs](String data){
      bool value = convertToNumeric<String,bool>(data);
      if(value){
        stairs.onDown2Up();
      }
    });

  });
  
  
  

}

// fauxmoESP fauxmo;
// AsyncWebServer server(80);
// DNSServer dns;
// void setup(){
//   Serial.begin(115200);
//   delay(50);
//   Serial.print(F("Welcome to ConnectedStair"));
// //   Serial.println(ESP.getFreeHeap(),DEC);

// //   const char* json = "{\"pin\":\"D8\",\"lightOffAfter\":5000,\"brightness\":35,\"LDRThreshold\":35,\"enableAlexa\":true,\"animator\":{\"effect\":\"normal\",\"speed\":30,\"transition\":300},\"stairstep\":[{\"pixel\":1,\"color\":[\"0xffffffff\",\"0x000000\"]},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1},{\"pixel\":1,\"color\":[\"0xffffffff\",\"0x000000\"]}]}";

// //   // if(stair.loadFromMemory(json.c_str())){
// //   // //   //  // stair.saveToFile();
// //   //  Serial.println("load OK");
// //   //  }else{
// //   //    Serial.println("load Error");
// //   //  }
// //   //    Serial.println(ESP.getFreeHeap(),DEC);

// // stair.loadFromMemory(json);
// //    stair.setParams();
// DNSServer dns;
//   AsyncWiFiManager  wifiManager(&server,&dns);


//   if (!wifiManager.autoConnect("AP-CONNNECTED-STAIRS")) {
//     Serial.println(F("failed to connect and hit timeout"));
//     delay(3000);
//     //reset and try again, or maybe put it to deep sleep
//     ESP.reset();
//     delay(5000);
//   }


// fauxmo.addDevice("light one");
//     fauxmo.addDevice("light two");
//     fauxmo.addDevice("light three");
//     fauxmo.addDevice("light four");

//     fauxmo.setPort(80); // required for gen3 devices
//     fauxmo.enable(true);

//     fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
//         Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
//     });
// }

// void loop(){

//       fauxmo.handle();

// }