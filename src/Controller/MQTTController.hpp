#pragma once

#include "IController.hpp"

#include <map>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

class MQTTController : public IController{

// wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
//   wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

        AsyncMqttClient mqttClient;
        // Ticker mqttReconnectTimer;

        bool isConnected;

        std::function<void()> onConnectedFunc;
        std::function<void()> onDisconnectedFunc;

        typedef std::function<void(String&)> rFunction;
        std::map<String, rFunction > rFunc;

        String host;
        IPAddress ipAddress;
        uint32_t port;
        String username;
        String password;
        String SSL;



    private:

        void connectToMqtt() {
            Serial.println(F("Connecting to MQTT..."));
            mqttClient.connect();
        }

        void onMqttConnect(bool sessionPresent) {
            isConnected=true;
            Serial.println(F("Connected to MQTT."));
            // Serial.print("Session present: ");
            // Serial.println(sessionPresent);
            onConnectedFunc();
            // uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
            // Serial.print("Subscribing at QoS 2, packetId: ");
            // Serial.println(packetIdSub);
            // mqttClient.publish("test/lol", 0, true, "test 1");
            // Serial.println("Publishing at QoS 0");
            // uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
            // Serial.print("Publishing at QoS 1, packetId: ");
            // Serial.println(packetIdPub1);
            // uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
            // Serial.print("Publishing at QoS 2, packetId: ");
            // Serial.println(packetIdPub2);
        }

        void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
            isConnected=false;
            Serial.println(F("Disconnected from MQTT."));
            onDisconnectedFunc();
            //TODO
            // if (WiFi.isConnected()) {
            //     mqttReconnectTimer.once(2, std::bind(&MQTTClient::connectToMqtt, this));
            // }
        }

        void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
            // Serial.println("Subscribe acknowledged.");
            // Serial.print("  packetId: ");
            // Serial.println(packetId);
            // Serial.print("  qos: ");
            // Serial.println(qos);
        }

        void onMqttUnsubscribe(uint16_t packetId) {
            // Serial.println("Unsubscribe acknowledged.");
            // Serial.print("  packetId: ");
            // Serial.println(packetId);
        }

        void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
            // Serial.println("Publish received.");
            // Serial.print("  topic: ");
            // Serial.println(topic);
            // Serial.print("  payload: ");
            // Serial.println(payload);
            // Serial.print("  qos: ");
            // Serial.println(properties.qos);
            // Serial.print("  dup: ");
            // Serial.println(properties.dup);
            // Serial.print("  retain: ");
            // Serial.println(properties.retain);
            // Serial.print("  len: ");
            // Serial.println(len);
            // Serial.print("  index: ");
            // Serial.println(index);
            // Serial.print("  total: ");
            // Serial.println(total);

            std::map<String, rFunction >::iterator it = rFunc.find(String(topic));
            if (it != rFunc.end()){
                // Serial.println("Publish received.");
                // Serial.print("  topic: ");
                // Serial.println(topic);
                // Serial.print("  payload: ");
                // Serial.println(payload);
                String sData(payload);
                it->second(sData);
            }

            ///rFunction func = rFunc.at(String(topic));
           // String sData = String(payload).substring(0,len);

            
        }

        void onMqttPublish(uint16_t packetId) {
            // Serial.println("Publish acknowledged.");
            // Serial.print("  packetId: ");
            // Serial.println(packetId);
        }

    public:

        static String confPath(){
            return "/mqttConf.json";
        }

        MQTTController(const char* name);

        void connect();

        void connect(String host,unsigned int port);

        void connect(String host,unsigned int port,String username, String password);
    
        void connect(IPAddress ip,unsigned int port);

        void connect(IPAddress ipAddress_,unsigned int port_,String username_, String password_);

        void onConnected(std::function<void()> f);

        void onDisconnected(std::function<void()> f);

        void on(String topic, rFunction f);
        
        bool deserialize(const JsonObject& root);

        bool loadFromMemory(const char* json);

        bool loadFromFile();

        void saveToFile();

        bool getStatus(){
            return isConnected;
        }

        void onLightOff(FuncVVCallback func){}
        void onColorChange(FuncVSCallback func){}
        void onBrightnessChange(FuncVUCCallback func){}
        void onGetConnectedStairsConf(FuncBSRefCallback func){}
        void onSetConnectedStairsConf(FuncBSCallback func){}
        void onGetMQTTConf(FuncBSRefCallback func){}
        void onSetMQTTConf(FuncBSCallback func){}
        void onGetNeoPixelConf(FuncBSRefCallback func){}
        void onSetNeoPixelConf(FuncBSCallback func){}


};