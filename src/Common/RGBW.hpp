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

        template<class T>
        T getRedColor() {
            return T(_r);
        }

        template<class T>
        T getGreenColor() {
            return T(_g);
        }

        template<class T>
        T getBlueColor() {
            return T(_b);
        }

        template<class T>
        T getWhiteColor() {
            return T(_w);
        }

        bool isFullBlack() const{
            if(_r == 0 && _g == 0 && _b == 0 && _w == 0)
                return true;
            return false;
        }
      
};

namespace color{

    inline int interpolate(int a, int b, int c, int d, double t, double s){
        return (int)(a*(1-t)*(1-s) + b*t*(1-s) + c*(1-t)*s + d*t*s);
    }


    inline RGBW getInterpolateColor( RGBW& a,  RGBW& b, RGBW& c,  RGBW& d, int w, int h, int x, int y){
        double t = double(x)/double(w);
        double s = double(y)/double(h);
        // int red = interpolate(a.getRedColor(), b.getRedColor(), c.getRedColor(), d.getRedColor(), t, s);
        // int green = interpolate(a.getGreenColor(), b.getGreenColor(), c.getGreenColor(), d.getGreenColor(), t, s);
        // int blue = interpolate(a.getBlueColor(), b.getBlueColor(), c.getBlueColor(), d.getBlueColor(), t, s);
        // int white = interpolate(a.getWhiteColor(), b.getWhiteColor(), c.getWhiteColor(), d.getWhiteColor(), t, s);

        //return RGBW(red,green,blue,white);
        return RGBW(0,0,0,0);

    }
}