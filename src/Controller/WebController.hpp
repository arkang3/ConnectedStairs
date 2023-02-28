#pragma once

#include "IController.hpp"
#include <ESPAsyncWebServer.h> 
#include <LittleFS.h>

class SSIDWifi{

    String _SSID;
    uint8_t _encryptionType;
    int32_t _RSSI;
    uint8_t *_BSSID;
    int32_t _channel;
    bool _isHidden;

    private:

        int32_t getRSSIasQuality(int RSSI) const {
            int32_t quality = 0;

            if (RSSI <= -100){
                quality = 0;
            }
            else if (RSSI >= -50){
                quality = 100;
            }else{
                quality = 2 * (RSSI + 100);
            }
            return quality;
        }

    public:
   
        SSIDWifi(String SSID, uint8_t encryptionType, int32_t RSSI, uint8_t *BSSID, int32_t channel, bool isHidden){
            _SSID = SSID;
            _encryptionType = encryptionType;
            _RSSI = RSSI;
            _BSSID = BSSID;
            _channel = channel;
            _isHidden = isHidden;

        }

        bool operator < (const SSIDWifi& other) const{
            return getRSSIasQuality(_RSSI) < other.getQuality();
        }

        bool operator == (const SSIDWifi& other) const{
            return _SSID == other.getSSID();
        }

        String getSSID() const { return _SSID;}
        int32_t getQuality() const { return getRSSIasQuality(_RSSI);}
};


class CaptiveRequestHandler : public AsyncWebHandler
{
    std::vector<SSIDWifi> _wifiSSIDs;

    Ticker _threadScan;
    bool _scanReady;

    Ticker _threadConnection;
    int _maxRetries;
    int _retries;

    Ticker _threadDNS;
    DNSServer dnsServer;
    const byte DNS_PORT = 53;

    bool _isConnected;

    bool scanNetworks(){

        wifi_ssid_count_t n = WiFi.scanComplete();
        _scanReady = false;
        if (n == WIFI_SCAN_FAILED)
        {
            Serial.println(F("scanNetworks returned: WIFI_SCAN_FAILED!"));
        }
        else if (n == WIFI_SCAN_RUNNING)
        {
                Serial.println(F("scanNetworks returned: WIFI_SCAN_RUNNING!"));
        }
        else if (n < 0)
        {
                Serial.println(F("scanNetworks failed with unknown error code!"));
        }
        else if (n == 0)
        {
                Serial.println(F("No networks found"));
            // page += F("No networks found. Refresh to scan again.");
        }
        else
        {
                Serial.println(F("Scan done"));
        }

        if(n>0){
            _wifiSSIDs.clear();
            String SSID;
            uint8_t encryptionType;
            int32_t RSSI;
            uint8_t *BSSID;
            int32_t channel;
            bool isHidden;

            for (wifi_ssid_count_t i = 0; i < n; i++){
                WiFi.getNetworkInfo(i,
                        SSID,
                        encryptionType,
                        RSSI,
                        BSSID,
                        channel,
                        isHidden);
                _wifiSSIDs.emplace_back(SSID,encryptionType,RSSI,BSSID,channel,isHidden);
            }

            std::sort(_wifiSSIDs.begin(), _wifiSSIDs.end());
            _wifiSSIDs.erase(std::unique(_wifiSSIDs.begin(), _wifiSSIDs.end()), _wifiSSIDs.end());    
            WiFi.scanDelete();
            _scanReady = true;
        }

        return _scanReady;
    }

    void connectToWifi(){
        _retries = 0;
        _threadConnection.attach_ms(500,[&](){
            Serial.print("count : ");Serial.println(_retries);
            if(_retries>=_maxRetries){
                _retries = 0;
                _threadConnection.detach();
                Serial.println(F("WiFi connection FAILED"));
                _isConnected = false;
                WiFi.mode(WIFI_AP_STA);
            }else{

                if (WiFi.status() == WL_CONNECTED) {
                    WiFi.persistent(false);
                    _retries = 0;
                    if(_threadDNS.active())
                        _threadDNS.detach();
                    _threadConnection.detach();
                    Serial.println(F("WiFi connected!"));
                    Serial.println("IP address: ");
                    Serial.println(WiFi.localIP());
                    ESP.restart();
                }
            }
            _retries++;
        });
    }

public:
    CaptiveRequestHandler(AsyncWebServer* server,String deviceName) {
        _maxRetries=15;
        _isConnected = false;

        Serial.println("No SSID saved!!!!!!!!!");
        String apName = deviceName;
        apName.toUpperCase();
        apName="AP-"+apName;
        Serial.print("AP Mode: ");
        Serial.println(apName);
        WiFi.softAP(apName);
        delay(500);
        Serial.print(F("AP IP address: "));
        Serial.println(WiFi.softAPIP());
        
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        if (!dnsServer.start(DNS_PORT, "*", WiFi.softAPIP())){
            Serial.println(F("Could not start Captive DNS Server!"));
        }

        _threadDNS.attach_ms(10,[&](){
            dnsServer.processNextRequest();
        });

        server->on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/styles.css","text/css");
        });
        server->on("/s.css", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/s.css","text/css");
        });
        server->on("/loader.css", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/loader.css","text/css");
        });
        server->on("/4/w3.css", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/4/w3.css","text/css");
        });
        server->on("/api.js", HTTP_GET, [](AsyncWebServerRequest * request) {
            request->send(LittleFS, "/api.js", "text/js");
        });

        // server->on("/scanNetworks", HTTP_GET, [this](AsyncWebServerRequest * request) {
        //     wifi_ssid_count_t n = WiFi.scanNetworks(true);
        //     if(_threadScan.active()){
        //         _threadScan.detach();
        //     }
        //     _threadScan.attach_ms(500,[&](){
        //         if(scanNetworks()){
        //             _threadScan.detach();
        //         }
        //     });
        //     request->send(200, "text/html", "ok");
        // });
        // server->on("/getWifiList", HTTP_GET, [this](AsyncWebServerRequest * request) {
        //     if(_scanReady){
        //         StaticJsonDocument<512> doc;
        //         JsonArray ssidsArray = doc.createNestedArray("ssidList");
        //         for(auto i: _wifiSSIDs){
        //             JsonObject ssid = ssidsArray.createNestedObject();
        //             ssid["ssid"] = i.getSSID();
        //             ssid["value"] = i.getSSID();
        //         }
        //         String output;
        //         serializeJson(doc, output);
        //         request->send(200, "text/html", output.c_str());
        //     }else{
        //         request->send(201, "text/html", "waiting");
        //     }
        // });

        server->on("/setWifiConf", HTTP_GET, [&](AsyncWebServerRequest * request) {
                
            Serial.println("/wifiConf");
            WiFi.mode(WIFI_STA);
            WiFi.persistent(true);

            if (request->hasParam("ssid") && request->hasParam("password") ){
                WiFi.begin(request->getParam("ssid")->value(),request->getParam("password")->value());
            }else{
                if (request->hasParam("ssid")){
                    WiFi.begin(request->getParam("ssid")->value());
                }else{
                    WiFi.begin("","");
                }
            }

            connectToWifi();
            request->send(200, "text/html", "waiting");
        });

    }
    virtual ~CaptiveRequestHandler() {}
    bool canHandle(AsyncWebServerRequest *request)
    {
        //request->addInterestingHeader("ANY");
        return true;
    }
    void handleRequest(AsyncWebServerRequest *request){
        request->send(LittleFS, "/apmode.html", "text/html");
    }
};

class WebController : public IController{

        // DNSServer dnsServer;
        // const byte DNS_PORT = 53;

        String _modelPath;
        String _indexPath;
        String _wifiPath;
        String _mqttPath;
        String _stairsPath;

        String _indexHTML;
        String _wifiHTML;
        String _mqttHTML;
        String _stairsHTML;

        std::vector<SSIDWifi> _wifiSSIDs;

        FuncBSCallback _funcStairsConf;
        FuncBSCallback _funcMQTTConf;
        FuncVVCallback _funcOnConnected;
        std::map<String,FuncBVCallback> _serviceMap;

        Ticker _threadConnection;
        int _maxRetries;
        int _retries;

        Ticker _threadScan;
        bool _scanReady;

    private:


        void reset(){
            WiFi.mode(WIFI_AP_STA); // cannot erase if not in STA mode !
            WiFi.persistent(true);
            #if defined(ESP8266)
            WiFi.disconnect(true);
            #else
            WiFi.disconnect(true, true);
            #endif
            WiFi.persistent(false);
        }

        String generateHTML(String& modelPath, String& placeholderPath){ 
            String model;
            String placeholder;
            FSStream::read(_modelPath,model);
            FSStream::read(placeholderPath,placeholder);
            model.replace("%PLACEHOLDER%",placeholder);
            return model;
        }

        bool scanNetworks(){

            wifi_ssid_count_t n = WiFi.scanComplete();
            _scanReady = false;
            if (n == WIFI_SCAN_FAILED)
            {
                Serial.println(F("scanNetworks returned: WIFI_SCAN_FAILED!"));
            }
            else if (n == WIFI_SCAN_RUNNING)
            {
                 Serial.println(F("scanNetworks returned: WIFI_SCAN_RUNNING!"));
            }
            else if (n < 0)
            {
                 Serial.println(F("scanNetworks failed with unknown error code!"));
            }
            else if (n == 0)
            {
                 Serial.println(F("No networks found"));
                // page += F("No networks found. Refresh to scan again.");
            }
            else
            {
                 Serial.println(F("Scan done"));
            }

            if(n>0){
                _wifiSSIDs.clear();
                String SSID;
                uint8_t encryptionType;
                int32_t RSSI;
                uint8_t *BSSID;
                int32_t channel;
                bool isHidden;
                    Serial.println("scan :");

                for (wifi_ssid_count_t i = 0; i < n; i++){
                    WiFi.getNetworkInfo(i,
                          SSID,
                          encryptionType,
                          RSSI,
                          BSSID,
                          channel,
                          isHidden);
                    _wifiSSIDs.emplace_back(SSID,encryptionType,RSSI,BSSID,channel,isHidden);
                }

                std::sort(_wifiSSIDs.begin(), _wifiSSIDs.end());
                _wifiSSIDs.erase(std::unique(_wifiSSIDs.begin(), _wifiSSIDs.end()), _wifiSSIDs.end());    
                WiFi.scanDelete();
                _scanReady = true;
            }

           return _scanReady;

        }

        void connectToWifi(){
            _retries = 0;
            _threadConnection.attach_ms(1000,[&](){
                Serial.print("count : ");Serial.println(_retries);
                if(_retries>=_maxRetries){
                    _retries = 0;
                    _threadConnection.detach();
                    Serial.println(F("WiFi connection FAILED"));
                    WiFi.mode(WIFI_AP_STA);
                }else{

                    if (WiFi.status() == WL_CONNECTED) {
                        WiFi.persistent(false);
                        _retries = 0;
                        _threadConnection.detach();
                        Serial.println(F("WiFi connected!"));
                        Serial.println("IP address: ");
                        Serial.println(WiFi.localIP());
                        _funcOnConnected();
                    }
                }
                _retries++;
            });
        }

    public:

        WebController(const char* name):IController(name){
            _modelPath = "template.min.html";
            _indexPath = "index.min.html";
            _wifiPath = "wifi.min.html";
            _mqttPath = "mqtt.min.html";
            _stairsPath = "stairs.min.html";
            _maxRetries=15;
        }

        void listen(AsyncWebServer* server){

            server->addHandler(new CaptiveRequestHandler(server,_deviceName)).setFilter(ON_AP_FILTER);

            server->on("/", HTTP_GET, [this](AsyncWebServerRequest * request) {
                request->send(200, "text/html",generateHTML(_modelPath,_indexPath));
            });
            server->on("/index.html", HTTP_GET, [this](AsyncWebServerRequest * request) {
                request->send(200, "text/html", generateHTML(_modelPath,_indexPath));
            });
            server->on("/wifi.html", HTTP_GET, [this](AsyncWebServerRequest * request) {
                request->send(200, "text/html", generateHTML(_modelPath,_wifiPath));
            });
            server->on("/mqtt.html", HTTP_GET, [this](AsyncWebServerRequest * request) {
                request->send(200, "text/html", generateHTML(_modelPath,_mqttPath));
            });
            server->on("/stairs.html", HTTP_GET, [this](AsyncWebServerRequest * request) {
                request->send(200, "text/html", generateHTML(_modelPath,_stairsPath));
            });


            server->on("/getStairsStatus", HTTP_GET, [&](AsyncWebServerRequest * request) {
                String jsonStatus = "{\"status\":\""+String(_serviceMap["getStairsStatus"]())+"\"}";
                request->send(200, "text/html", jsonStatus);
            });
            server->on("/getStairsConfig", HTTP_GET, [this](AsyncWebServerRequest * request) {
                String path = "stairsConf.json";
                String fileContent;
                if(FSStream::read(path,fileContent)){
                    Serial.println("stairs found");
                    request->send(200, "text/html", fileContent);
                }else{
                    Serial.println("no stairs config");
                    request->send(200, "text/html", "KO");
                }
            });
            server->on("/setStairsConf", HTTP_GET, [&](AsyncWebServerRequest * request) {
                Serial.println("/setStairsConf :");
                Serial.println(request->getParam("JSON")->value());
                if(_funcStairsConf(request->getParam("JSON")->value()))
                    request->send(200, "text/html", "OK");
                else
                    request->send(200, "text/html", "KO");

            });

            server->on("/getMQTTConfig", HTTP_GET, [this](AsyncWebServerRequest * request) {
                String path = "mqttConf.json";
                String fileContent;
                if(FSStream::read(path,fileContent)){
                    Serial.print("mqtt found");
                    request->send(200, "text/html", fileContent);
                }else{
                    Serial.print("no mqtt config");
                    request->send(200, "text/html", "KO");
                }
            });
            server->on("/setMQTTConf", HTTP_GET, [&](AsyncWebServerRequest * request) {
                Serial.println("/setMQTTConf :");
                Serial.println(request->getParam("JSON")->value());
                if(_funcMQTTConf(request->getParam("JSON")->value()))
                    request->send(200, "text/html", "OK");
                else
                    request->send(200, "text/html", "KO");
            });
            server->on("/getMQTTStatus", HTTP_GET, [&](AsyncWebServerRequest * request) {
                String jsonStatus = "{\"status\":\""+String(_serviceMap["getMQTTStatus"]())+"\"}";
                request->send(200, "text/html", jsonStatus);
            });

           
            server->on("/scanNetworks", HTTP_GET, [this](AsyncWebServerRequest * request) {
                //wifi_ssid_count_t n = WiFi.scanNetworks(true);
                if(_threadScan.active()){
                    _threadScan.detach();
                }
                _threadScan.attach_ms(500,[&](){
                    if(scanNetworks()){
                        _threadScan.detach();
                    }
                });
                request->send(200, "text/html", "ok");
            });
            server->on("/getWifiList", HTTP_GET, [this](AsyncWebServerRequest * request) {
                if(_scanReady){
                    StaticJsonDocument<512> doc;
                    JsonArray ssidsArray = doc.createNestedArray("ssidList");
                    for(auto i: _wifiSSIDs){
                        JsonObject ssid = ssidsArray.createNestedObject();
                        ssid["ssid"] = i.getSSID();
                        ssid["value"] = i.getSSID();
                    }
                    String output;
                    serializeJson(doc, output);
                    request->send(200, "text/html", output.c_str());
                }else{
                    request->send(201, "text/html", "waiting");
                }
            });
            // server->on("/setWifiConf", HTTP_GET, [&](AsyncWebServerRequest * request) {
            //     Serial.println("/wifiConf");
            //     WiFi.mode(WIFI_STA);
            //     WiFi.persistent(true);
            //     wl_status_t statut;
            //     if (request->hasParam("ssid") && request->hasParam("password") ){
            //         WiFi.begin(request->getParam("ssid")->value(),request->getParam("password")->value());
            //     }else{
            //         if (request->hasParam("ssid")){
            //             WiFi.begin(request->getParam("ssid")->value());
            //         }else{
            //             WiFi.begin("","");
            //         }
            //     }
            //     connectToWifi();
            //     request->send(200, "text/html", "waiting");
            // });

            
            

            
            
            


            // server->on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
            //     request->send(LittleFS, "/styles.css","text/css");
            // });
            // server->on("/s.css", HTTP_GET, [](AsyncWebServerRequest *request){
            //     request->send(LittleFS, "/s.css","text/css");
            // });
            // server->on("/loader.css", HTTP_GET, [](AsyncWebServerRequest *request){
            //     request->send(LittleFS, "/loader.css","text/css");
            // });
            // server->on("/4/w3.css", HTTP_GET, [](AsyncWebServerRequest *request){
            //     request->send(LittleFS, "/4/w3.css","text/css");
            // });
            // server->on("/api.js", HTTP_GET, [](AsyncWebServerRequest * request) {
            //     request->send(LittleFS, "/api.js", "text/js");
            // });

            server->on("/action", HTTP_GET, [&](AsyncWebServerRequest * request) {
                Serial.println("new action");

                if (request->hasParam("JSON")){

                    Serial.println(request->getParam("JSON")->value());
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
                                _fnColor("0x000000");
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

        void onLightOff(IController::FuncVVCallback fnLightOff){
            _fnLightOff = fnLightOff;
        }

        void onColorChange(IController::FuncVSCallback fnColor){
            _fnColor = fnColor;
        }

        void onBrightnessChange(IController::FuncVUCCallback fnBrightness){
            _fnBrightness = fnBrightness;
        }

        void onConnectedStairsConf(FuncBSCallback func) {
            _funcStairsConf = func;
        }
      
        void onMQTTConf(FuncBSCallback func) {
            _funcMQTTConf = func;
        }

        void onConnected(FuncVVCallback func){
            _funcOnConnected = func;
        }

        void autoConnect(){
            // reset();
            WiFi.mode(WIFI_STA);
            WiFi.begin();
            delay(500);
            connectToWifi();
        }

        void setServiceStatus(String service, FuncBVCallback func){
            _serviceMap[service] = func;
        }
       
};