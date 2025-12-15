#pragma once

#include "Core/Application.h"

extern Ballistic::Application* Ballistic::CreateApplication();

int main(int argc, char** argv) {
    gl::Buffer buf;
    Ballistic::Application* app = Ballistic::CreateApplication();
    app->run();
    delete app;
}