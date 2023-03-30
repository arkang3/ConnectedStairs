#include "WifiConnector.hpp"
#include "CaptivePortal.hpp"

#include <ArduinoJson.h>
#include <AsyncElegantOTA.h>

#include "../fsStream/FSStreamInternal.hpp"

int32_t SSIDWifi::getRSSIasQuality(int RSSI) const {
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

SSIDWifi::SSIDWifi(String SSID, uint8_t encryptionType, int32_t RSSI, uint8_t *BSSID, int32_t channel){
    _SSID = SSID;
    _encryptionType = encryptionType;
    _RSSI = RSSI;
    _BSSID = BSSID;
    _channel = channel;
}

bool SSIDWifi::operator < (const SSIDWifi& other) const{
    return getRSSIasQuality(_RSSI) < other.getQuality();
}

bool SSIDWifi::operator == (const SSIDWifi& other) const{
    return _SSID == other.getSSID();
}

String SSIDWifi::getSSID() const { return _SSID;}

int32_t SSIDWifi::getQuality() const { return getRSSIasQuality(_RSSI);}

bool WifiConnector::scanNetworks(){

    _wifiSSIDs.clear();
    uint16_t n = WiFi.scanNetworks(false);

    // int16_t n = WiFi.scanComplete();
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
        String SSID;
        uint8_t encryptionType;
        int32_t RSSI;
        uint8_t *BSSID;
        int32_t channel;

        for (int16_t i = 0; i < n; i++){
            WiFi.getNetworkInfo(i,
                    SSID,
                    encryptionType,
                    RSSI,
                    BSSID,
                    channel);
            _wifiSSIDs.emplace_back(SSID,encryptionType,RSSI,BSSID,channel);
        }
//TODO: doublon
        std::sort(_wifiSSIDs.begin(), _wifiSSIDs.end());
        _wifiSSIDs.erase(std::unique(_wifiSSIDs.begin(), _wifiSSIDs.end()), _wifiSSIDs.end());    
        WiFi.scanDelete();
        _scanReady = true;
    }

    return _scanReady;

}

WifiConnector::WifiConnector(const char* apName){
    _apName = apName;
    _maxRetries = 15;
    _isConnected = false;
    _mustRestart = false;
    _scanReady = false;
    WiFi.begin();
    WiFi.disconnect();
}

void WifiConnector::autoConnect(AsyncWebServer& server, std::function<void()> onConnected,uint8_t maxRetries){

    _funcOnConnected = onConnected;
    _maxRetries = maxRetries;

    server.on("/wifiConnector.js", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String file;
        FSStreamInternal::read(String("/wifiConnector.js").c_str(),file);
        request->send(200, "text/javascript", file);
    });

    server.on("/scanNetworks", HTTP_GET, [this](AsyncWebServerRequest * request) {
        if(_threadScan){
            _threadScan->join();
            _threadScan.reset();
        }
        _threadScan = std::make_shared<std::thread>(&WifiConnector::scanNetworks, this);
        request->send(200, "text/html", "ok");
    });

    server.on("/getWifiList", HTTP_GET, [this](AsyncWebServerRequest * request) {
        if(_scanReady){
            _scanReady=false;
            StaticJsonDocument<512> doc;
            JsonArray ssidsArray = doc.createNestedArray("ssidList");
            for(auto i: _wifiSSIDs){
                Serial.print( i.getSSID());
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

    server.on("/setWifiConf", HTTP_GET, [&](AsyncWebServerRequest * request) {
        Serial.println("/wifiConf");
        WiFi.mode(WIFI_STA);
        WiFi.persistent(true);
        wl_status_t statut;
        if (request->hasParam("ssid") && request->hasParam("password") ){
            WiFi.begin(request->getParam("ssid")->value().c_str(),request->getParam("password")->value().c_str());
        }else{
            if (request->hasParam("ssid")){
                WiFi.begin(request->getParam("ssid")->value().c_str());
            }else{
                WiFi.begin("","");
            }
        }
        _mustRestart=true;
        _threadConnection = std::make_shared<std::thread>(&WifiConnector::tryToConnect, this);
        request->send(200, "text/html", "waiting");
    });

     //reset();
    WiFi.mode(WIFI_STA);
    //WiFi.begin();
    WiFi.begin("Lelahel","K6jt3qIM4igV");
    WiFi.setSleep(false);
    Serial.println("autoconnect");

    _threadConnection = std::make_shared<std::thread>(&WifiConnector::tryToConnect, this);
    _threadConnection->join();
    _threadConnection.reset();
    
    if(_isConnected){
        AsyncElegantOTA.begin(&server);  
        _funcOnConnected();
    }else{
        server.addHandler(new CaptivePortal(server)).setFilter(ON_AP_FILTER);
        startCaptivePortal();
    }
}

void WifiConnector::tryToConnect(){

    for(uint8_t retries=0;retries<_maxRetries;retries++){
        if (WiFi.status() == WL_CONNECTED) {
            _isConnected=true;
            WiFi.persistent(false);
            retries = _maxRetries;
            Serial.println(F("WiFi connected!"));
            Serial.println(F("IP address: "));
            Serial.println(WiFi.localIP());
            if(!MDNS.begin(_apName.c_str())) {
                Serial.println("Error starting mDNS");
            }
            if(_mustRestart) ESP.restart();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

}

void WifiConnector::startCaptivePortal(){

    Serial.println(F("WiFi connection FAILED!!!!"));
    WiFi.mode(WIFI_AP_STA);
    String apName = _apName;
    apName.toUpperCase();
    apName="AP-"+apName;
    Serial.print(F("AP Mode: "));
    Serial.println(apName);
    WiFi.softAP(apName.c_str());
    Serial.print(F("AP IP address: "));
    Serial.println(WiFi.softAPIP());
    
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    if (!dnsServer.start(DNS_PORT, "*", WiFi.softAPIP())){
        Serial.println(F("Could not start Captive DNS Server!"));
    }

    // _threadDNS = std::make_shared<std::thread>([&](){
    //     dnsServer.processNextRequest();
    //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // }, this);
  
}