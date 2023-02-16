


class Threadable{

        Ticker thread;

    public:

        Threadable(unsigned int milliseconds,std::function<void()> func)
        {
            thread.attach_ms(milliseconds,[&](){
                func();
            });

        }

        void join(){

            thread.attach_ms(milliseconds,[&](){
                func();
            });

        }

        void stop(){
            thread.detach();
        }

        void 


};
