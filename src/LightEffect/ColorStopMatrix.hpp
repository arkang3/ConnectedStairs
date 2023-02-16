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

        std::map<float, std::vector<ColorStop> > _matrix;
        std::vector<ColorStopSquareView> _squareView;

    public:

        ColorStopMatrix(){}

        void clear(){
            _matrix.clear();
            _squareView.clear();
        }

        void addLine(float index, std::vector<ColorStop>& colorStopLine){
            _matrix[index] = colorStopLine;
            Serial.print("at: ");
            Serial.println(index);
            Serial.print("--> ");
            for(int i=0;i< colorStopLine.size();i++){
                Serial.print(colorStopLine[i].getColor());
                Serial.print(" - ");
            }
            Serial.println(".");

        }

        const std::map<float, std::vector<ColorStop> >& getRawData(){
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

                    Serial.println("valid square :");
                    Serial.println(upL->getColor());
                    Serial.println(upR->getColor());
                    Serial.println(loL->getColor());
                    Serial.println(loR->getColor());

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

                bool res = false;

                float wh = it.at(1)->getX() - it.at(0)->getX();
                

                bool t= ( x >= it.at(0)->getX() && y >= it.at(0)->getY() && x <= it.at(0)->getX()+wh && y <= it.at(0)->getY()+wh);


                // Serial.print("pF(");Serial.print(x);Serial.print(",");Serial.print(y);Serial.println(")");

                // Serial.print("p0(");Serial.print(it.at(0)->getX());Serial.print(",");Serial.print(it.at(0)->getY());Serial.println(")");
                // Serial.print("p1(");Serial.print(it.at(1)->getX());Serial.print(",");Serial.print(it.at(1)->getY());Serial.println(")");
                // Serial.print("p2(");Serial.print(it.at(2)->getX());Serial.print(",");Serial.print(it.at(2)->getY());Serial.println(")");
                // Serial.print("p3(");Serial.print(it.at(3)->getX());Serial.print(",");Serial.print(it.at(3)->getY());Serial.println(")");

                if(t){
                    //Serial.println("found");
                    squareView = it;
                    return true;
                }
                    
                 //{
                //     return bilinearInterpolateColor(it.at(0)->getColor(),it.at(1)->getColor(),it.at(2)->getColor(),it.at(3)->getColor(),x,y );
                // }


            }
            return false;
            
        }




};