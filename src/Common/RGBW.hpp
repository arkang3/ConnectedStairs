#pragma once

class RGBW{

        unsigned char _r;
        unsigned char _g;
        unsigned char _b;
        unsigned char _w;

    public:

        enum RGBW_CHANNEL{
            R,
            G,
            B,
            W
        };
       
        RGBW(){}
        RGBW(unsigned char r, unsigned char g, unsigned char b, unsigned char w):_r(r),_g(g),_b(b),_w(w){}
        RGBW(String color){
            if(color.length()==10){
                unsigned int hexValue;
                std::stringstream sstream;
                sstream << std::hex << color.c_str();
                sstream >> hexValue;
                _r = ((hexValue >> 24) & 0xFF) ;
                _g = ((hexValue >> 16) & 0xFF) ;
                _b = ((hexValue >> 8) & 0xFF) ;  
                _w = ((hexValue) & 0xFF) ;  
            }
            if(color.length()==8){
                unsigned int hexValue;
                std::stringstream sstream;
                sstream << std::hex << color.c_str();
                sstream >> hexValue;
                _r = ((hexValue >> 16) & 0xFF) ;// Extract the GG byte
                _g = ((hexValue >> 8) & 0xFF) ;  
                _b = ((hexValue) & 0xFF) ;  
                _w = 0;
            }
            

        }

        RGBW(const RGBW& rgbw) = default;
        
        uint32_t getColor() const{
              return ((uint32_t)_w << 24) | ((uint32_t)_r << 16) | ((uint32_t)_g << 8) | _b;
        }

        void getStrColor(){
            Serial.print("getStrColor : ");
            Serial.print(_r);
            Serial.print(" ");
            Serial.print(_g);
            Serial.print(" ");
            Serial.print(_b);
            Serial.println(" ");
        }

        template<class T>
        T getRedColor() const{
            return T(_r);
        }

        template<class T>
        T getGreenColor() const{
            return T(_g);
        }

        template<class T>
        T getBlueColor() const{
            return T(_b);
        }

        template<class T>
        T getWhiteColor() const{
            return T(_w);
        }

        bool isBlack() const{
            if(_r == 0 && _g == 0 && _b == 0 && _w == 0)
                return true;
            return false;
        }
      
};
