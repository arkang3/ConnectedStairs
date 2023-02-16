#pragma once

#include "IAnimator.hpp"
#include <Ticker.h>

//only 1 color in this mode
class NormalAnimator : public IAnimator{

        Ticker transitionTicker;
        Ticker offTicker;
        int executionCount;
        int maxExecutionCount;

        unsigned int startTimer;
        unsigned int resetTimer;

        unsigned int transition;

        rgbw startColorIn;
        rgbw endColorIn;

        rgbw startColorOut;
        rgbw endColorOut;

        bool restartTimer;



    public :

        NormalAnimator(){
            transition =500;
            startColorIn.r=255;
            startColorIn.g=0;
            startColorIn.b=0;
            endColorIn.r=0;
            endColorIn.g=0;
            endColorIn.b=255;
        }

        NormalAnimator(JsonObject& object){
            transition =500;
            // int error;
            // error = getValueFromJSON<int, unsigned int>(object["speed"] | -1 , speed, -1);
            // if(!error) {
            //     Serial.println("NormalAnimator speed error parsing");
            //     speed=0;
            // }
            // error = getValueFromJSON<int, unsigned int>(object["transition"] | -1 , transition, -1);
            // if(!error) {
            //     Serial.println("NormalAnimator transition error parsing");
            //     transition=0;
            // }
          
        }

        // void prepareColor(std::vector<PixelStep>& pixels,rgbw& s, rgbw& e){

        //     for(int i=0;i<pixels.size();i++){
        //         float percent = float(i*100.f/(pixels.size()))/100.f
        //         computeGradient(s,e,percent);
        //     }
            
        // }

        void runDown2Up(Adafruit_NeoPixel& neopixels, std::vector<PixelStep>& pixels){
            executionCount=0;
            maxExecutionCount=pixels.size()+1;

            transitionTicker.attach_ms(transition,[&](){
                //isRunning=true;
                executionCount++;

                if(executionCount>=maxExecutionCount){
                    transitionTicker.detach();
                  //  isRunning=false;
                }else{
                    Serial.println("run");
                    
                    unsigned int id = executionCount-1;
                    PixelStep subpixels = pixels[id];
                    Serial.println(subpixels.begin());
                    Serial.println(subpixels.end());
                    float percent = float(id*100.f/(pixels.size()))/100.f;
                    rgbw color = computeGradient(startColorIn,endColorIn,percent);

                    for(unsigned int i=subpixels.begin();i<subpixels.end();i++)
                        neopixels.setPixelColor(i, color.r,color.g,color.b);
                    
                    neopixels.show();
                }
            });
        }

        void runUp2Down(Adafruit_NeoPixel& neopixels, std::vector<PixelStep>& pixels){
            executionCount=0;
            maxExecutionCount=3;

            transitionTicker.attach_ms(transition,[&](){
                //isRunning=true;
                executionCount++;

                if(executionCount>=maxExecutionCount){
                    transitionTicker.detach();
                  //  isRunning=false;
                }else{
                    Serial.println("run");
                    PixelStep subpixels = pixels[executionCount-1];
                    for(unsigned int i=subpixels.begin();i<subpixels.end();i++)
                        neopixels.setPixelColor(i, 255,255,255);
                    
                    neopixels.show();
                }
            });
        }


        void run(Adafruit_NeoPixel& neopixels, std::vector<PixelStep>& pixels){
        
            executionCount=0;
            maxExecutionCount=pixels.size()+1;

            transitionTicker.attach_ms(transition,[&](){
            
                executionCount++;

                if(executionCount>=maxExecutionCount){
                    transitionTicker.detach();
                }else{
                    Serial.println("run");
                    PixelStep subpixels = pixels[executionCount-1];
                    for(unsigned int i=subpixels.begin();i<subpixels.end();i++)
                        neopixels.setPixelColor(i, 255,255,255);
                    
                    neopixels.show();
                }
            });
        }

        void startOffTimer(Adafruit_NeoPixel& neopixels, std::vector<PixelStep>& pixels,StairDir& currentDir){
            offTicker.detach();
            offTicker.once_ms(10000+resetTimer-startTimer,[&](){
                        runOut(neopixels,pixels,currentDir);
            });
        }

        void runIn(Adafruit_NeoPixel& neopixels, std::vector<PixelStep>& pixels,StairDir& currentDir){
            Serial.print("runIn ");

            if(currentDir==DOWN2UP_RUNNING || currentDir==DOWN2UP_FINISHED){
                Serial.print("restart.... ");
                resetTimer = millis();
                startOffTimer(neopixels,pixels,currentDir);
            }

            if(currentDir==StairDir::DOWN2UP){
                Serial.print("start...");
                resetTimer= startTimer = millis();

                executionCount=0;
                maxExecutionCount=pixels.size()+1;

                transitionTicker.attach_ms(transition,[&](){
                    currentDir=DOWN2UP_RUNNING;

                    executionCount++;

                    if(executionCount>=maxExecutionCount){
                        currentDir=DOWN2UP_FINISHED;
                        transitionTicker.detach();
                        Serial.print("waiting....");
                        startOffTimer(neopixels,pixels,currentDir);
                    }else{
                        unsigned int id = executionCount-1;
                        const PixelStep& subpixels = pixels[id];
                        float percent = float(id*100.f/(pixels.size()))/100.f;
                        rgbw color = computeGradient(startColorIn,endColorIn,percent);
                        // Serial.println(subpixels.begin());
                        // Serial.println(subpixels.end());
                        for(unsigned int i=subpixels.begin();i<subpixels.end();i++)
                            neopixels.setPixelColor(i, color.r,color.g,color.b);
                        
                        neopixels.show();
                    }
                });
            }

            if(currentDir==StairDir::UP2DOWN){
                executionCount=0;
                maxExecutionCount=pixels.size()+1;

                transitionTicker.attach_ms(transition,[&](){
                    currentDir=UP2DOWN_RUNNING;
               
                    executionCount++;

                    if(executionCount>=maxExecutionCount){
                        currentDir=UP2DOWN_FINISHED;
                        transitionTicker.detach();
                        offTicker.once_ms(5000,[&](){
                            Serial.println("offffff");
                            runOut(neopixels,pixels,currentDir);
                        });
                    }else{
                        Serial.println("run");
                        unsigned int id = executionCount-1;
                        const PixelStep& subpixels = pixels[(pixels.size()-1)-(id)];
                        float percent = float(id*100.f/(pixels.size()))/100.f;
                        rgbw color = computeGradient(startColorIn,endColorIn,percent);
                        Serial.println(subpixels.begin());
                        Serial.println(subpixels.end());
                        for(unsigned int i=subpixels.begin();i<subpixels.end();i++)
                            neopixels.setPixelColor(i, 255,255,255);
                        
                        neopixels.show();
                    }
                });
            }
        }

        void runOut(Adafruit_NeoPixel& neopixels, std::vector<PixelStep>& pixels,StairDir& currentDir){
            Serial.print("runOut ");

            // for(unsigned int i=0;i<pixels.size();i++){
            //     const PixelStep& subpixels = pixels[i];
            //     for(unsigned int i=subpixels.begin();i<subpixels.end();i++)
            //         neopixels.setPixelColor(i, 0,0,0);
            // }
            // neopixels.show();
       
        }
};
