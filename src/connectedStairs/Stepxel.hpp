#pragma once 

class Stepxel{

        uint16_t _uid;
        typedef std::pair<uint16_t, uint16_t> Interval;
        Interval _step;
        uint16_t _size;
        bool _isDirty;
        bool _status;
        uint16_t _seek;

    private:

        void defaultConfig(){
            _uid= std::numeric_limits<uint16_t>::max();
            _step = std::make_pair(0,0);
            _size = 0;
            _status = false;
        }

    public:

        Stepxel(const uint16_t& uid,const uint16_t& begin,const uint16_t& end){
            _uid=uid;
            _step = std::make_pair(begin,end);
            _size = end - begin;
            _isDirty = false;
            _status = false;
            _seek = 0;
        }

        Stepxel(const JsonObject& object,uint16_t& offset){
            _isDirty = false;
            bool error=false;
            unsigned short nPixel=0;
            error = ArduinoJson::extends::getValueFromJSON<int16_t, uint16_t>(object["pixels"] | 0 , nPixel, 0);
            if(!error){
                _isDirty = true;
            }

            _seek=0;
            error = ArduinoJson::extends::getValueFromJSON<int16_t, uint16_t>(object["seek"] | 0 , _seek, 0);

            if(_isDirty){
                defaultConfig();
            }else{
                _step = std::make_pair(offset+_seek,offset+nPixel+_seek);
                _size = nPixel ;
                offset = offset + nPixel + _seek;
                _status = false;
            }

            // Serial.println("stepxel: ");
            // Serial.print("_size: ");Serial.println(_size);
            // Serial.print("_step: ");Serial.print(_step.first); Serial.print(" : ");Serial.println(_step.second);
        }

        void serialize(JsonObject& object){
            object["pixels"] = _size;
            if(_seek)
                object["seek"] = _seek;
        }

        bool isDirty(){
            return _isDirty;
        }

        Stepxel& operator=(const Stepxel& other){
            _uid = other.uid();
            _step = std::make_pair(other.begin(),other.end());
            _size = other.size();
            _seek = other.seek();
            return *this;
        }

        uint16_t uid() const {
            return _uid;
        }
        
        uint16_t begin() const {
            return _step.first;
        }

        uint16_t end() const {
            return _step.second;
        }

        uint16_t size() const {
            return _size;
        }

         uint16_t seek() const {
            return _seek;
        }

        void setStatus(bool value){
            _status=value;
        }

        bool status() const{
            return _status;
        }


};