#pragma once 

#define FS_NO_GLOBALS //allow spiffs to coexist with SD card, define BEFORE including FS.h
#include <FS.h> //spiff file system
#include "LittleFS.h"
#include "FSStreamMode.hpp"

class FSStreamInternal{
    
    public:

        static fs::LittleFSFS& get(){ return LittleFS;}
        static String getDeviceName(){ return "FSStreamInternal";}

         static void begin(){
            if (!LittleFS.begin(true)) {
                Serial.print(F("An error has occurred while mounting "));
                Serial.println(getDeviceName());
            }else{
                Serial.print(getDeviceName());
                Serial.println(F(" mounted successfully"));
            }
        }

         static bool exists(const char* path){
            return LittleFS.exists(path);
        }

        static bool open(fs::File& file, const char* path, uint8_t mode=FSStreamMode::READ){
            switch(mode){
                case FSStreamMode::READ:
                    file = LittleFS.open(path,"r");
                    return true;
                    break;
            }
            return false;
        }

        static bool read(const char* path,String& fileContent){
            fs::File file;
            fileContent = String();
            if(open(file,path)){
                fileContent = file.readString();
                file.close();
                return true;
            }
            return false;
        }
        
};
