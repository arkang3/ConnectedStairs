
#include "MQTTController.hpp"

#include <StreamUtils.h>

#include "../FSStream/FSStreamInternal.hpp"
#include "../common/Utils.hpp"

MQTTController::MQTTController(const char* name):IController(name){

    isConnected=false;
    mqttClient.onConnect(std::bind(&MQTTController::onMqttConnect, this, std::placeholders::_1));
    mqttClient.onDisconnect(std::bind(&MQTTController::onMqttDisconnect, this, std::placeholders::_1));
    mqttClient.onSubscribe(std::bind(&MQTTController::onMqttSubscribe, this, std::placeholders::_1,std::placeholders::_2));
    mqttClient.onUnsubscribe(std::bind(&MQTTController::onMqttUnsubscribe, this, std::placeholders::_1));
    mqttClient.onMessage(std::bind(&MQTTController::onMqttMessage, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6));
    mqttClient.onPublish(std::bind(&MQTTController::onMqttPublish, this, std::placeholders::_1));

}

void MQTTController::connect(){
    mqttClient.setServer(host.c_str(), port);
    connectToMqtt();
}

void MQTTController::connect(String host_,unsigned int port_){
    host=host_;
    port=port_;
    mqttClient.setServer(host.c_str(), port);
    connectToMqtt();
}

void MQTTController::connect(String host_,unsigned int port_,String username_, String password_){
    host=host_;
    port=port_;
    username=username_;
    password=password_;
    mqttClient.setCredentials(username.c_str(),password.c_str());
    mqttClient.setServer(host.c_str(), port);
    connectToMqtt();
}

void MQTTController::connect(IPAddress ipAddress_,unsigned int port_){
    ipAddress=ipAddress_;
    port=port_;
    mqttClient.setServer(ipAddress, port);
    connectToMqtt();
}

void MQTTController::connect(IPAddress ipAddress_,unsigned int port_,String username_, String password_){
    ipAddress=ipAddress_;
    port=port_;
    username=username_;
    password=password_;
    mqttClient.setCredentials(username.c_str(),password.c_str());
    mqttClient.setServer(ipAddress, port);
    connectToMqtt();
}

void MQTTController::onConnected(std::function<void()> f){
    onConnectedFunc = f;
}

void MQTTController::onDisconnected(std::function<void()> f){
    onDisconnectedFunc = f;
}

void MQTTController::on(String topic, rFunction f){
    // Serial.print(F("Subscribing: "));
    // Serial.println(topic);
    if(topic=="PIRDown") mqttClient.subscribe(pirDown, 0); rFunc[pirDown] = f;
    if(topic=="PIRUp") mqttClient.subscribe(pirUp, 0); rFunc[pirUp] = f;
    if(topic=="LDR") mqttClient.subscribe(ldr, 0); rFunc[ldr] = f;

}

bool MQTTController::loadFromFile(){
            
    fs::File file;
    if(FSStreamInternal::open(file,confPath().c_str(),FSStreamMode::READ)){
        ReadBufferingStream bufferedFile(file, 64); 
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, bufferedFile);
        const JsonObject& root = doc.as<JsonObject>();
        if(deserialize(root)){
            return true;
        }else{
            Serial.println(F("deserialize load error"));
            return false;
        }
    }else{
        Serial.println(F("FSStream::load error"));
        return false;
    }
}

bool MQTTController::loadFromMemory(const char* json){
   
    DynamicJsonDocument doc(1024);
    if(!ArduinoJson::extends::strToJson(json,doc))
        return false;

    const JsonObject& root = doc.as<JsonObject>();

    if( deserialize(root)){
        return true;
    }else{
        Serial.println(F("deserialize load error"));
        return false;
    }
}

bool MQTTController::deserialize(const JsonObject& root){
    
    bool error;

    error = ArduinoJson::extends::getValueFromJSON<const char*, String>(root["server"] | ((const char*)(NULL)) , host, ((const char*)(NULL)));
    if (!error) {
        Serial.print(F("host error parsing"));
        return error;
    }

    error = ArduinoJson::extends::getValueFromJSON<unsigned int, unsigned int>(root["port"] | 0 , port, 0 );
    if (!error) {
        Serial.print(F("port error parsing"));
        port = 1883;
    }

    error = ArduinoJson::extends::getValueFromJSON<const char*, String>(root["pirDown"] | ((const char*)(NULL)) , pirDown, ((const char*)(NULL)));
    if (!error) {
        Serial.print(F("pirDown error parsing"));
        return error;
    }

    error = ArduinoJson::extends::getValueFromJSON<const char*, String>(root["pirUP"] | ((const char*)(NULL)) , pirUP, ((const char*)(NULL)));
    if (!error) {
        Serial.print(F("pirUP error parsing"));
        return error;
    }

    error = ArduinoJson::extends::getValueFromJSON<const char*, String>(root["ldr"] | ((const char*)(NULL)) , ldr, ((const char*)(NULL)));
    if (!error) {
        Serial.print(F("ldr error parsing"));
        return error;
    }

    Serial.println(host);
    Serial.println(port);

    Serial.println(pirDown);
    Serial.println(pirUP);
    Serial.println(ldr);

    // String host_="";
    // error = getValueFromJSON<const char*, String>(root["host"] | ((const char*)(NULL)) , host_, ((const char*)(NULL)));
    // if (error) {
    //     Serial.print("pin error parsing");
    //     return error;
    // }
    connect();
    return true;

}

void MQTTController::saveToFile(){
            
    fs::File file;
    if(FSStreamInternal::open(file,confPath().c_str(),FSStreamMode::WRITE)){
        WriteBufferingStream buffered(file, 64);
        DynamicJsonDocument doc(1024);

        doc["server"]   = host;
        //doc["ipAddress"]   = ipAddress;
        doc["port"]   = port;
        doc["username"]   = username;
        doc["password"]   = password;
        //doc["SSL"]   = SSL;

        serializeJson(doc, buffered);
        buffered.flush();
    }else{
        Serial.print(F("error open MQTT write file"));
    }
    file.close();
   
}
