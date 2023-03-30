#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <DNSServer.h> 
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>

class SSIDWifi{

        String _SSID;
        uint8_t _encryptionType;
        int32_t _RSSI;
        uint8_t *_BSSID;
        int32_t _channel;   

        int32_t getRSSIasQuality(int RSSI) const;

    public:
   
        SSIDWifi(String SSID, uint8_t encryptionType, int32_t RSSI, uint8_t *BSSID, int32_t channel);

        bool operator < (const SSIDWifi& other) const;

        bool operator == (const SSIDWifi& other) const;

        String getSSID() const ;
        int32_t getQuality() const;
};

class WifiConnector{

        String _apName;
        uint8_t _maxRetries;
        bool _isConnected;
        bool _mustRestart;
        bool _scanReady;

        std::vector<SSIDWifi> _wifiSSIDs;
        
        std::shared_ptr<std::thread> _threadConnection;
        std::shared_ptr<std::thread> _threadDNS;
        std::shared_ptr<std::thread> _threadScan;

        DNSServer dnsServer;
        const byte DNS_PORT = 53;

        std::function<void()> _funcOnConnected;

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

        void tryToConnect();
        void startCaptivePortal();

    public:

        WifiConnector(const char* apName);

        bool scanNetworks();

        void autoConnect(AsyncWebServer& server, std::function<void()> onConnected,uint8_t maxRetries=15);

        void disconnect(){}


};