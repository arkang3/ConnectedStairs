#include <StreamUtils.h>

#include "MQTTClient.hpp"
#include "../Common/FSStream.hpp"
#include "../Common/Utils.hpp"

MQTTClient::MQTTClient(){

    isConnected=false;
    mqttClient.onConnect(std::bind(&MQTTClient::onMqttConnect, this, std::placeholders::_1));
    mqttClient.onDisconnect(std::bind(&MQTTClient::onMqttDisconnect, this, std::placeholders::_1));
    mqttClient.onSubscribe(std::bind(&MQTTClient::onMqttSubscribe, this, std::placeholders::_1,std::placeholders::_2));
    mqttClient.onUnsubscribe(std::bind(&MQTTClient::onMqttUnsubscribe, this, std::placeholders::_1));
    mqttClient.onMessage(std::bind(&MQTTClient::onMqttMessage, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6));
    mqttClient.onPublish(std::bind(&MQTTClient::onMqttPublish, this, std::placeholders::_1));

}

void MQTTClient::connect(){
    mqttClient.setServer(host.c_str(), port);
    connectToMqtt();
}

void MQTTClient::connect(String host_,unsigned int port_){
    host=host_;
    port=port_;
    mqttClient.setServer(host.c_str(), port);
    connectToMqtt();
}

void MQTTClient::connect(String host_,unsigned int port_,String username_, String password_){
    host=host_;
    port=port_;
    username=username_;
    password=password_;
    mqttClient.setCredentials(username.c_str(),password.c_str());
    mqttClient.setServer(host.c_str(), port);
    connectToMqtt();
}

void MQTTClient::connect(IPAddress ipAddress_,unsigned int port_){
    ipAddress=ipAddress_;
    port=port_;
    mqttClient.setServer(ipAddress, port);
    connectToMqtt();
}

void MQTTClient::connect(IPAddress ipAddress_,unsigned int port_,String username_, String password_){
    ipAddress=ipAddress_;
    port=port_;
    username=username_;
    password=password_;
    mqttClient.setCredentials(username.c_str(),password.c_str());
    mqttClient.setServer(ipAddress, port);
    connectToMqtt();
}

void MQTTClient::onConnected(std::function<void()> f){
    onConnectedFunc = f;
}

void MQTTClient::onDisconnected(std::function<void()> f){
    onDisconnectedFunc = f;
}

void MQTTClient::on(String topic, rFunction f){
    Serial.print(F("Subscribing: "));
    Serial.println(topic);
    mqttClient.subscribe(topic.c_str(), 0);
    rFunc[topic] = f;
}

bool MQTTClient::loadFromFile(String confPath){
            
    File file;
    if(FSStream::open(confPath,file,"r")){
        ReadBufferingStream bufferedFile(file, 64); 
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, bufferedFile);
        const JsonObject& root = doc.as<JsonObject>();
        if(deserialize(root)){
            connect();
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

bool MQTTClient::loadFromMemory(const char* json){
   
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

bool MQTTClient::deserialize(const JsonObject& root){
    
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

    error = ArduinoJson::extends::getValueFromJSON<const char*, String>(root["username"] | ((const char*)(NULL)) , username, ((const char*)(NULL)));
    if (!error) {
        Serial.print(F("username error parsing"));
        return error;
    }

    error = ArduinoJson::extends::getValueFromJSON<const char*, String>(root["password"] | ((const char*)(NULL)) , password, ((const char*)(NULL)));
    if (!error) {
        Serial.print(F("password error parsing"));
        return error;
    }

    // String host_="";
    // error = getValueFromJSON<const char*, String>(root["host"] | ((const char*)(NULL)) , host_, ((const char*)(NULL)));
    // if (error) {
    //     Serial.print("pin error parsing");
    //     return error;
    // }

    return true;

}

void MQTTClient::saveToFile(String confPath){
            
    File file;
    if(FSStream::open(confPath,file,"w")){
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
   
}
