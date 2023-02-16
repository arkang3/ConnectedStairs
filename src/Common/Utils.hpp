#pragma once

#include <sstream>

namespace ArduinoJson{
    namespace extends{
        template<class T, class U>
        inline bool getValueFromJSON(T jsonValue, U& value, T errorValue) {
            if (jsonValue == errorValue)
                return false;
            U t(jsonValue);
            value = t;
            return true;
        }

        inline bool strToJson(const char* json,DynamicJsonDocument& doc){
            
            DeserializationError DesError = deserializeJson(doc, json);
            if(DesError) {
                Serial.println(F("Failed to parse config file"));
                Serial.println(DesError.f_str());
                return false;
            }
            return true;
        }
    }
}



template<class T,class U>
inline U convertToNumeric(T& data){
    U value;
    std::stringstream sstream;
    sstream << data.c_str();
    sstream >> value;
    return value;
}

