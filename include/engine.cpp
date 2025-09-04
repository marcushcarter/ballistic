#include "engine.hpp"
#include "engine_default.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <source_location>

static void BE_Message(int severity, const std::string& module, const std::string& message, const std::string& file = "", int line = 0, const std::source_location& loc = std::source_location::current()) {
    std::string displayFile;
    int displayLine;

    if (!file.empty() && line != 0) {
        displayFile = file;
        displayLine = line;
    } else {
        displayFile = loc.file_name();
        displayLine = loc.line();
    }

    static std::string lastMsg = "";
    static int repeatCount = 0;

    std::string colorCode, label;
    switch (severity) {
        case 0: colorCode = "\033[37m"; label = "[INFO]"; break;
        case 1: colorCode = "\033[35m"; label = "[WARNING]"; break;
        case 2: colorCode = "\033[31m"; label = "[ERROR]"; break;
        case 3: colorCode = "\033[91m"; label = "[FATAL]"; break;
        default: colorCode = "\033[37m"; label = "[INFO]"; break;
    }

    std::ostringstream ss;
    ss << colorCode << label << "\033[0m "
       << displayFile << ":" << displayLine << " -> "
       << module << ": " << message;

    std::string fullMsg = ss.str();

    if (fullMsg == lastMsg) {
        repeatCount++;
        std::cout << "\r" << fullMsg << " (x" << repeatCount + 1 << ")\033[0m";
        std::cout.flush();
    } else {
        if (repeatCount > 0) std::cout << std::endl;
        lastMsg = fullMsg;
        repeatCount = 0;
        std::cerr << fullMsg << "\033[0m" << std::endl;
    }

    if (severity >= 3) exit(1);
}

// ========================================================================

float BE_FrameTime::update() {
    currentTime = std::chrono::high_resolution_clock::now();
    dt = std::chrono::duration<float>(currentTime - previousTime).count();
    previousTime = currentTime;

    frameCount++;
    frameCountFPS++;
    fpsTimer += dt;

    if (fpsTimer >= 1.0f) {
        fps = frameCountFPS / fpsTimer;
        ms = 1000.0f / fps;

        fpsHistory[fpsHistoryIndex] = fps;
        fpsHistoryIndex = (fpsHistoryIndex + 1) % FPS_HISTORY_COUNT;
        if (fpsHistoryCount < FPS_HISTORY_COUNT)
            fpsHistoryCount++;

        frameCountFPS = 0;
        fpsTimer = 0.0f;
    }

    return dt;

}

// ========================================================================

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    BE_Engine* engine = static_cast<BE_Engine*>(glfwGetWindowUserPointer(window));
    if (!engine) return;

    engine->width = width;
    engine->height = height;
    glViewport(0, 0, width, height);

    // BE_Message(0, "ENGINE", "Framebuffer resized", __FILE__, __LINE__);
}

static BE_Engine* g_boundEngine = nullptr;

BE_Engine::BE_Engine(const std::string& t, int w, int h, const std::source_location& loc) : title(t), width(w), height(h), running(true), window(nullptr) {
    
    if (!glfwInit()) {
        BE_Message(3, "ENGINE", "Failed to initialize GLFW", loc.file_name(), loc.line());
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        BE_Message(3, "ENGINE", "Failed to create GLFW window", loc.file_name(), loc.line());
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        BE_Message(3, "ENGINE", "Failed to initialize GLAD", loc.file_name(), loc.line());
    }
    
    // resources

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwShowWindow(window);

}

BE_Engine::~BE_Engine() {
    if (window) glfwDestroyWindow(window);
    if (g_boundEngine == this) g_boundEngine = nullptr;
    glfwTerminate();
}

void BE_Engine::bind() { g_boundEngine = this; }

bool BE_Engine::isRunning() const {
    return running && !glfwWindowShouldClose(window);
}

void BE_Engine::closeWindow() {
    running = false;
}

void BE_Engine::beginFrame() {

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    glfwPollEvents();
    // inputs
    frameTime.update();
    // update audio engine
    //set listener position to camera
}

void BE_Engine::beginRender() {
    glViewport(0, 0, width, height);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT || GL_DEPTH_BUFFER_BIT);
}

void BE_Engine::endFrame() { glfwSwapBuffers(window); }
