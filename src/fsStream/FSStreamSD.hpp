#pragma once 

#include <SD.h>
#include <SPI.h>
#include "FSStreamMode.hpp"
class FSStreamSD{
    
    public:

        static String getDeviceName(){ return "FSStreamSD";}

        static void begin(){
            if (!SD.begin()) {
                Serial.print(F("An error has occurred while mounting "));
                Serial.println(getDeviceName());
            }else{
                Serial.print(getDeviceName());
                Serial.println(F(" mounted successfully"));
            }
        }

         static bool exists(const char* path){
            return SD.exists(path);
        }

        static bool open(fs::File& file, const char* path, uint8_t mode=FSStreamMode::READ){
            file = SD.open(path,"r");
            // switch(mode){
            //     case FSStreamMode::READ:
            //         file = SD.open(path,FILE_READ);
                    return true;
            //         break;
            // }
        }

        static bool read(const char* path,String& fileContent){
            fs::File file;
            fileContent = String();
            if(open(file,path)){
                fileContent = file.readString();
                file.close();
                return true;
            }
            file.close();
            return false;
        }


       
        
};