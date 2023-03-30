#include "CaptivePortal.hpp"
#include "../fsStream/FSStreamInternal.hpp"

CaptivePortal::CaptivePortal(AsyncWebServer& server) {
    

    server.on("/wifiConnector.js", HTTP_GET, [this](AsyncWebServerRequest * request) {
        String fileContent;
        FSStreamInternal::read(String("/wifiConnector.js").c_str(),fileContent);
        request->send(200, "text/js", fileContent);
    });

}

CaptivePortal::~CaptivePortal() {}

void CaptivePortal::connect(){
    // WifiConnector::connect(_deviceName,true);
}

bool CaptivePortal::canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
}

void CaptivePortal::handleRequest(AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html",apmode_html_gz, apmode_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}
