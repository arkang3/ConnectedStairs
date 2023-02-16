#pragma once

#include "LittleFS.h"

class FSStream{

    public:

        static void begin(){
            if (!LittleFS.begin()) {
                Serial.println(F("An error has occurred while mounting LittleFS"));
            }
            Serial.println(F("LittleFS mounted successfully"));
        }

        static bool open(String& path,File& file,const char* mode){

            file = LittleFS.open(path.c_str(),mode);
            if(!file || file.isDirectory()){
                Serial.println(F("- failed to open file for reading"));
                return false;
            }
            return true;
        }


        static bool read(const char* path,String& fileContent){
            File file = LittleFS.open(path,"r");
            if(!file || file.isDirectory()){
                Serial.println(F("- failed to open file for reading"));
                fileContent = String();
                return false;
            }
            fileContent = file.readString();
            return true;
        }

        static bool read(String& path,String& fileContent){
           // Serial.printf("Reading file: %s\r\n"), path.c_str();

            return read(path.c_str(), fileContent);

            // File file = LittleFS.open(path.c_str(),"r");
            // if(!file || file.isDirectory()){
            //     Serial.println(F("- failed to open file for reading"));
            //     fileContent = String();
            //     return 1;
            // }
            
            // fileContent = file.readString();
            // while(file.available()){
            //     fileContent = file.readStringUntil('\n');
            //     break;     
            // }
            // return 0;
        }

        static int write(String& path, String& fileContent){
           // Serial.printf("Writing file: %s\r\n"), path.c_str();

            File file = LittleFS.open(path.c_str(), "w");
            if(!file){
                Serial.println(F("- failed to open file for writing"));
                return 1;
            }
            if(file.print(fileContent)){
                file.flush();
                file.close();
                Serial.println(F("- file written"));
                return 0;
            } else {
                Serial.println(F("- frite failed"));
                return 1;
            }
        }

};