#pragma once

#include "Core/Application.h"

extern Ballistic::Application* Ballistic::CreateApplication();

int main(int argc, char** argv) {
    Ballistic::Application* app = Ballistic::CreateApplication();
    app->run();
    delete app;
}