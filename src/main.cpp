#include "include/engine.hpp"

#include <iostream>

int main() {

    BE_Engine engine("My Engine");
    engine.bind();

    while(engine.isRunning()) {
        engine.beginFrame();

        engine.beginRender();
        engine.endFrame();
    }

    engine.~BE_Engine();
    return 0;
}