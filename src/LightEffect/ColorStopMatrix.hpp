#pragma once


class ColorStop{

        float _x;
        float _y;
        String _color;

    public:

        ColorStop(){}

        ColorStop(float x, float y, String color){
            _x = x;
            _y = y;
            _color = color;
        }

        float getX(){return _x;}
        float getY(){return _y;}
        String getColor(){return _color;}


};

class ColorStopSquareView{

        std::array<ColorStop*,4> _square;

    public:

        ColorStopSquareView(){}
        ColorStopSquareView(ColorStop* upperL, ColorStop* upperR, ColorStop* lowerL, ColorStop* lowerR){
            _square[0] = upperL;
            _square[1] = upperR;
            _square[2] = lowerL;
            _square[3] = lowerR;
        }

        ColorStop* at(int i){
            return _square[i];
        }


};

class ColorStopMatrix{

        std::map<int, std::vector<ColorStop> > _matrix;
        std::vector<ColorStopSquareView> _squareView;
        float weight;

    public:

        ColorStopMatrix(){
            weight = 100.f;
        }

        void clear(){
            _matrix.clear();
            _squareView.clear();
        }

        void addLine(int index, std::vector<ColorStop>& colorStopLine){
            _matrix[index] = colorStopLine;
            // Serial.print("at: ");
            // Serial.println(index);
            // Serial.print("--> ");
            // for(int i=0;i< colorStopLine.size();i++){
            //     Serial.print(colorStopLine[i].getColor());
            //     Serial.print(" - ");
            // }
            // Serial.println(".");
        }

        const std::map<int, std::vector<ColorStop> >& getRawData(){
            return _matrix;
        }

        void buildGradientSquare(){

            for(auto iter = _matrix.begin(); iter != std::prev(_matrix.end()); iter++){
            
                auto lineUp = iter;
                iter = std::next(iter,1);
                auto lineDown = iter; 
            
                auto iter2 = lineUp->second.begin();
                auto iter3 = lineDown->second.begin();
                for(; iter2 != std::prev(lineUp->second.end()) && iter3 != std::prev(lineDown->second.end()) ; iter2++, iter3++){
                    auto upL = iter2;
                    iter2 = std::next(iter2,1);
                    auto upR = iter2;
                    
                    auto iter3 = lineDown->second.begin();
                    auto loL = iter3;
                    iter3 = std::next(iter3,1);
                    auto loR = iter3;
                    
                    _squareView.push_back({&(*upL),&(*upR),&(*loL),&(*loR)});

                    // Serial.println("valid square :");
                    // Serial.print(upL->getX());Serial.print(" ");Serial.println(upL->getY());
                    // Serial.print(upR->getX());Serial.print(" ");Serial.println(upR->getY());
                    // Serial.print(loL->getX());Serial.print(" ");Serial.println(loL->getY());
                    // Serial.print(loR->getX());Serial.print(" ");Serial.println(loR->getY());

                    // Serial.println(upL->getColor());
                    // Serial.println(upR->getColor());
                    // Serial.println(loL->getColor());
                    // Serial.println(loR->getColor());

                    iter2 = std::next(iter2,-1);
                    iter3 = std::next(iter3,-1);
                    
                }
                iter = std::next(iter,-1);
            
            }
        }

        // RGBW bilinearInterpolateColor(RGBW upperL,RGBW upperR, RGBW lowerL, RGBW lowerR, float x, float y){
        //     RGBW x1 = interpolateColor(upperL, upperR, x);
        //     RGBW x2 = interpolateColor(lowerL, lowerR, x);
        //     return interpolateColor(x1, x2, y);
        // }

        bool getColorStopSquareView(float x, float y,ColorStopSquareView& squareView){


            for(auto it: _squareView){

                float w = it.at(1)->getX() - it.at(0)->getX();
                float h = it.at(2)->getY() - it.at(1)->getY();
                
               // Serial.println("getColor4 ");
               // Serial.print(x);Serial.print(" ");Serial.println(y);
                bool isInRect= ( x >= it.at(0)->getX() && y >= it.at(0)->getY() && x <= it.at(0)->getX()+w && y <= it.at(0)->getY()+h);


                // Serial.print("pF(");Serial.print(x);Serial.print(",");Serial.print(y);Serial.println(")");

                // Serial.print("p0(");Serial.print(it.at(0)->getX());Serial.print(",");Serial.print(it.at(0)->getY());Serial.println(")");
                // Serial.print("p1(");Serial.print(it.at(1)->getX());Serial.print(",");Serial.print(it.at(1)->getY());Serial.println(")");
                // Serial.print("p2(");Serial.print(it.at(2)->getX());Serial.print(",");Serial.print(it.at(2)->getY());Serial.println(")");
                // Serial.print("p3(");Serial.print(it.at(3)->getX());Serial.print(",");Serial.print(it.at(3)->getY());Serial.println(")");

                if(isInRect){
                    //TODO: erreur 3 gradient vertical
                    // Serial.println("in square :");
                    // Serial.println(it.at(0)->getColor());
                    // Serial.println(it.at(1)->getColor());
                    // Serial.println(it.at(2)->getColor());
                    // Serial.println(it.at(3)->getColor());
                    squareView = it;
                    return true;
                }
                    
                 //{
                //     return bilinearInterpolateColor(it.at(0)->getColor(),it.at(1)->getColor(),it.at(2)->getColor(),it.at(3)->getColor(),x,y );
                // }


            }
            Serial.println("not foun squareView");
            return false;
            
        }

        RGBW interpolateColor(RGBW c1,RGBW c2,float fraction){

            float red1 = c1.getRedColor<float>()/255.f;
            float green1 = c1.getGreenColor<float>()/255.f;
            float blue1 = c1.getBlueColor<float>()/255.f;
            float white1 = c1.getWhiteColor<float>()/255.f;

            float red2 = c2.getRedColor<float>()/255.f;
            float green2 = c2.getGreenColor<float>()/255.f;
            float blue2 = c2.getBlueColor<float>()/255.f;
            float white2 = c2.getWhiteColor<float>()/255.f;

            float deltaRed = red2 - red1;
            float deltaGreen = green2 - green1;
            float deltaBlue = blue2 - blue1;
            float deltaWhite = white2 - white1;

            float red = red1 + (deltaRed * fraction);
            float green = green1 + (deltaGreen * fraction);
            float blue = blue1 + (deltaBlue * fraction);
            float white = white1 + (deltaWhite * fraction);

            if(red>1.f) red=1.f;
            if(red<0.f) red=0.f;

            if(green>1.f) green=1.f;
            if(green<0.f) green=0.f;

            if(blue>1.f) blue=1.f;
            if(blue<0.f) blue=0.f;

            if(white>1.f) white=1.f;
            if(white<0.f) white=0.f;

            return RGBW(red*255, green*255, blue*255, white*255);        
        }

        RGBW bilinearInterpolateColor(RGBW upperL,RGBW upperR, RGBW lowerL, RGBW lowerR, float x, float y){
            RGBW x1 = interpolateColor(upperL, upperR, x);
            RGBW x2 = interpolateColor(lowerL, lowerR, x);
            return interpolateColor(x1, x2, y);
            return RGBW();
        }

        RGBW getGradientColor(float x, float y){
            //TODO: test gradient complexe
            // float yy = float(y)/(_stepxels.size()-1);
            // float xx = float(x)/(_stepxels[y].size()-1);

            // Serial.print("xx :");Serial.print(xx);
            // Serial.print(" yy :");Serial.println(yy);

            if(x<0.f) x=0.f;
            if(x>1.f) x=1.f; 

            if(y<0.f) y=0.f;
            if(y>1.f) y=1.f;

            ColorStopSquareView squareView;
            bool err = getColorStopSquareView(x,y,squareView);

            if(err){
                return bilinearInterpolateColor(squareView.at(0)->getColor(),squareView.at(1)->getColor(),squareView.at(2)->getColor(),squareView.at(3)->getColor(),x,y );
            }

            Serial.print("errrrrr");
            return RGBW(0,0,0,0);
        }

        void configure(const JsonArray& object){

            bool error;
            bool ret = true;
            for (JsonVariant arrayMaxGradient : object) {
                float y;
                error = ArduinoJson::extends::getValueFromJSON<double, float>(arrayMaxGradient["at"] |  -1.0 , y, -1.0);
                if (!error) {
                    Serial.println(F("at error parsing")); 
                    float t = arrayMaxGradient["at"];
                    Serial.println(t);
                    ret = false;
                }

                //at == y

                std::vector<ColorStop> lineGradient;
                const JsonArray& lineGradientArray = arrayMaxGradient["lineGradient"];
                for (JsonVariant arrayLineGradient : lineGradientArray) {
                    float x;
                    error = ArduinoJson::extends::getValueFromJSON<float,float>(arrayLineGradient["at"] | -1 , x, -1);
                    if(!error) {
                        ret = false;
                    }
                    String color;
                    error = ArduinoJson::extends::getValueFromJSON<const char*, String>(arrayLineGradient["color"] | ((const char*)(NULL)) , color, ((const char*)(NULL)));
                    if(!error) {
                        ret = false;
                    }

                    lineGradient.emplace_back(x,y,color);
                }
                addLine( y*weight,lineGradient);
            }
            if(ret)buildGradientSquare();
        }

        void serialize(JsonArray& object){

            for(auto it : getRawData()){
                
                JsonObject r = object.createNestedObject();
                double atY = float(it.first) / weight;
                r["at"] = ArduinoJson::extends::round2(atY);
                JsonArray l = r.createNestedArray("lineGradient");
                for(auto it2 : it.second){
                    JsonObject u = l.createNestedObject();
                    u["at"] = it2.getX();
                    u["color"] = it2.getColor();
                }
            }
        }

};