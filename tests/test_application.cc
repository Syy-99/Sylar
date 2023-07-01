#include "../sylar/application.h"
#include <iostream>
int main(int argc, char** argv) {
    sylar::Application app;
    if(app.init(argc, argv)) {
        std::cout<<"??"<<std::endl;
        return app.run();
    }

    return 0;
}