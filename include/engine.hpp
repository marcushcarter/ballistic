#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <source_location>
#include <array>
#include <chrono>

class BE_FrameTime {
private:
    std::chrono::high_resolution_clock::time_point previousTime;
    std::chrono::high_resolution_clock::time_point currentTime;
public:
    
    static constexpr int FPS_HISTORY_COUNT = 100;

    float update();

    float dt = 0.0f;
    int frameCount = 0;
    int frameCountFPS = 0;
    float fpsTimer = 0.0f;
    float fps = 0.0f;
    float ms = 0.0f;

    std::array<float, FPS_HISTORY_COUNT> fpsHistory{};
    int fpsHistoryIndex = 0;
    int fpsHistoryCount = 0;

};

class BE_Engine {
private:

public:
    GLFWwindow* window;
    std::string title;
    bool running = true;
    int width;
    int height;

    BE_FrameTime frameTime;

    /** hello */
    BE_Engine(const std::string& title = "", int width = 1440, int height = 900, const std::source_location& loc = std::source_location::current());
    ~BE_Engine();
    void bind();

    bool isRunning() const;
    void closeWindow();

    void beginFrame();
    void beginRender();
    void endFrame();

};
