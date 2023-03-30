#pragma once

template<typename T>
class ColorStop{

        float _x;
        float _y;
        T _color;
    public:

        ColorStop(){}

        ColorStop(float x, float y, String color){
            _x = x;
            _y = y;
            _color = T(color);
        }

        float getX(){return _x;}
        float getY(){return _y;}
        T getColor() const {return _color;}

};

template<typename T>
class ColorStopSquareView{

        std::array<ColorStop<T>*,4> _square;

    public:

        ColorStopSquareView(){}
        ColorStopSquareView(ColorStop<T>* upperL, ColorStop<T>* upperR, ColorStop<T>* lowerL, ColorStop<T>* lowerR){
            _square[0] = upperL;
            _square[1] = upperR;
            _square[2] = lowerL;
            _square[3] = lowerR;
        }

        ColorStop<T>* at(int i){
            return _square[i];
        }


};

template<typename T>
class ColorStopMatrix{

        std::map<int, std::vector<ColorStop<T> > > _matrix;
        std::vector<ColorStopSquareView<T>> _squareView;
        float weight;

    public:

        ColorStopMatrix(){
            weight = 100.f;
        }

        void clear(){
            _matrix.clear();
            _squareView.clear();
        }

        void addLine(int index, std::vector<ColorStop<T>>& colorStopLine){
            _matrix[index] = colorStopLine;
        }

        const std::map<int, std::vector<ColorStop<T>> >& getRawData(){
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

                    iter2 = std::next(iter2,-1);
                    iter3 = std::next(iter3,-1);
                    
                }
                iter = std::next(iter,-1);
            
            }
        }

        bool getColorStopSquareView(float x, float y,ColorStopSquareView<T>& squareView){

            for(auto it: _squareView){

                auto i0 = it.at(0);
                auto i1 = it.at(1);
                auto i2 = it.at(2);
                if(x >= i0->getX() && y >= i0->getY()){
                    float w = i1->getX() - i0->getX();
                    float h = i2->getY() - i1->getY();

                    if(x <= i0->getX()+w && y <= i0->getY()+h){
                        squareView = it;
                        return true;
                    }
                
                }
            }
            Serial.println("not foun squareView");
            return false;
            
        }


        T interpolateColor(const T& c1,const T& c2,float fraction){
            return FastMath::lerp8by8(c1,c2,fraction*255);
        }

        T bilinearInterpolateColor(const T& upperL,const T& upperR, const T& lowerL, const T& lowerR, float x, float y){
            T x1 = interpolateColor(upperL, upperR, x);
            T x2 = interpolateColor(lowerL, lowerR, x);
            return interpolateColor(x1, x2, y);
        }

        T getGradientColor(float x, float y){
           
            // if(x<0.f) x=0.f;
            // if(x>1.f) x=1.f; 

            // if(y<0.f) y=0.f;
            // if(y>1.f) y=1.f;

            // Serial.print("xx :");Serial.print(x);
            // Serial.print(" yy :");Serial.println(y);

            ColorStopSquareView<T> squareView;
            bool err = getColorStopSquareView(x,y,squareView);

            if(err){
                return bilinearInterpolateColor(squareView.at(0)->getColor(),squareView.at(1)->getColor(),squareView.at(2)->getColor(),squareView.at(3)->getColor(),x,y );
            }

            Serial.print("errrrrr");
            return T();
        }

        void configure(const JsonArray& object){

            bool error;
            bool ret = true;
            for (JsonVariant arrayMaxGradient : object) {
                float y;
                error = ArduinoJson::extends::getValueFromJSON<double, float>(arrayMaxGradient["at"] |  -1.0 , y, -1.0);
                if (!error) {
                    Serial.println(F("at error parsing")); 
                    ret = false;
                }

                //at == y

                std::vector<ColorStop<T>> lineGradient;
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
                    Serial.print(x);Serial.print(" ");Serial.print(y);Serial.print(" ");
                    Serial.println(color);
                    lineGradient.emplace_back(x,y,color);
                }
                addLine( y*weight,lineGradient);
            }
            if(ret){
                Serial.print("buildGradientSquare");
                buildGradientSquare();

                for(float x =0;x<1.f;x+=0.1){
                    T a = getGradientColor(x,0);
                    Serial.print(a.getR());Serial.print(" ");
                    Serial.print(a.getG());Serial.print(" ");
                    Serial.print(a.getB());Serial.print(" ");
                    Serial.println();

                }
            }
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
                    u["color"] = it2.getColor().toHex();
                }
            }
        }

};