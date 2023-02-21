#pragma once 

#include "RGBW.hpp"

class Stepxel{

        unsigned int _uid;
        typedef std::pair<unsigned int, unsigned int> Interval;
        Interval _step;
        unsigned int _size;
        bool _isDirty;
        bool _status;

    private:

        void defaultConfig(){
            _uid= std::numeric_limits<unsigned int>::max();
            _step = std::make_pair(0,0);
            _size = 0;
            _status = false;
        }

    public:

        Stepxel(unsigned int uid,unsigned int begin, unsigned int end,RGBW color){
            _uid=uid;
            _step = std::make_pair(begin,end);
            _size = end - begin;
            _isDirty = false;
            _status = false;
        }

        Stepxel(const JsonObject& object,unsigned int& offset){
            _isDirty = false;
            bool error=false;
            unsigned int nPixel=0;
            error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(object["pixels"] | 0 , nPixel, 0);
            if(!error){
                _isDirty = true;
            }

            unsigned int startAt=0;
            error = ArduinoJson::extends::getValueFromJSON<int, unsigned int>(object["startAt"] | 0 , startAt, 0);

            if(_isDirty){
                defaultConfig();
            }else{
                _step = std::make_pair(offset+startAt,offset+nPixel+startAt);
                _size = nPixel ;
                offset = offset + nPixel + startAt;
                _status = false;
            }

            // Serial.println("stepxel: ");
            // Serial.print("_size: ");Serial.println(_size);
            // Serial.print("_step: ");Serial.print(_step.first); Serial.print(" : ");Serial.println(_step.second);
        }

        void serialize(JsonObject& object){
            object["pixels"] = _size;
        }

        bool isDirty(){
            return _isDirty;
        }

        Stepxel& operator=(const Stepxel& other){
            _uid = other.uid();
            _step = std::make_pair(other.begin(),other.end());
            _size = other.size();

            return *this;
        }

        unsigned int uid() const {
            return _uid;
        }
        
        unsigned int begin() const {
            return _step.first;
        }

        unsigned int end() const {
            return _step.second;
        }

        unsigned int size() const {
            return _size;
        }

        void setStatus(bool value){
            _status=value;
        }

        bool status() const{
            return _status;
        }


};