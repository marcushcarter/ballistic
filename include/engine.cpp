#include "engine.hpp"
#include "engine_default.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
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

BE_VAO::BE_VAO() { glGenVertexArrays(1, &ID); }

BE_VAO::~BE_VAO() { glDeleteVertexArrays(1, &ID); }

void BE_VAO::bind() { glBindVertexArray(ID); }

void BE_VAO::unbind() { glBindVertexArray(0); }

// ========================================================================

BE_VBO::BE_VBO(GLfloat* vertices, GLsizeiptr size) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

BE_VBO::BE_VBO(const std::vector<BE_Vertex>& vertices) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(BE_Vertex), vertices.data(), GL_STATIC_DRAW);
}

BE_VBO::~BE_VBO() { glDeleteBuffers(1, &ID); }

void BE_VBO::bind() { glBindBuffer(GL_ARRAY_BUFFER, ID); }

void BE_VBO::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

void BE_VBO::linkVertexAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    bind();
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    unbind();
}

// ========================================================================

BE_EBO::BE_EBO(GLuint* indices, GLsizeiptr size) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
}

BE_EBO::BE_EBO(const std::vector<GLuint>& indices) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

BE_EBO::~BE_EBO() { glDeleteBuffers(1, &ID); }

void BE_EBO::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); }

void BE_EBO::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

// ========================================================================

std::string BE_Shader::getFileContents(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filename << "\n";
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void BE_Shader::getCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n";
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "SHADER_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n";
        }
    }
}

GLuint BE_Shader::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    switch (type) {
        case GL_VERTEX_SHADER:   getCompileErrors(shader, "VERTEX"); break;
        case GL_FRAGMENT_SHADER: getCompileErrors(shader, "FRAGMENT"); break;
        case GL_GEOMETRY_SHADER: getCompileErrors(shader, "GEOMETRY"); break;
        case GL_COMPUTE_SHADER:  getCompileErrors(shader, "COMPUTE"); break;
    }
    return shader;
}

BE_Shader::BE_Shader(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const std::string& computePath)
    : name(shaderName.empty() ? "new shader" : shaderName) {

    GLuint vertexShader = 0, fragmentShader = 0, geometryShader = 0, computeShader = 0;

    if (!vertexPath.empty()) vertexShader = compileShader(GL_VERTEX_SHADER, getFileContents(vertexPath));
    if (!fragmentPath.empty()) fragmentShader = compileShader(GL_FRAGMENT_SHADER, getFileContents(fragmentPath));
    if (!geometryPath.empty()) geometryShader = compileShader(GL_GEOMETRY_SHADER, getFileContents(geometryPath));
    if (!computePath.empty()) computeShader = compileShader(GL_COMPUTE_SHADER, getFileContents(computePath));

    ID = glCreateProgram();
    if (vertexShader) glAttachShader(ID, vertexShader);
    if (fragmentShader) glAttachShader(ID, fragmentShader);
    if (geometryShader) glAttachShader(ID, geometryShader);
    if (computeShader) glAttachShader(ID, computeShader);
    glLinkProgram(ID);
    getCompileErrors(ID, "PROGRAM");

    if (vertexShader) glDeleteShader(vertexShader);
    if (fragmentShader) glDeleteShader(fragmentShader);
    if (geometryShader) glDeleteShader(geometryShader);
    if (computeShader) glDeleteShader(computeShader);

}

BE_Shader::BE_Shader(const std::string& shaderName, const std::string* vertexSource, const std::string* fragmentSource, const std::string* geometrySource, const std::string* computeSource)
    : name(shaderName.empty() ? "new shader" : shaderName) {

    GLuint vertexShader = 0, fragmentShader = 0, geometryShader = 0, computeShader = 0;

    if (vertexSource) vertexShader = compileShader(GL_VERTEX_SHADER, *vertexSource);
    if (fragmentSource) fragmentShader = compileShader(GL_FRAGMENT_SHADER, *fragmentSource);
    if (geometrySource) geometryShader = compileShader(GL_GEOMETRY_SHADER, *geometrySource);
    if (computeSource) computeShader = compileShader(GL_COMPUTE_SHADER, *computeSource);

    ID = glCreateProgram();
    if (vertexShader) glAttachShader(ID, vertexShader);
    if (fragmentShader) glAttachShader(ID, fragmentShader);
    if (geometryShader) glAttachShader(ID, geometryShader);
    if (computeShader) glAttachShader(ID, computeShader);
    glLinkProgram(ID);
    getCompileErrors(ID, "PROGRAM");

    if (vertexShader) glDeleteShader(vertexShader);
    if (fragmentShader) glDeleteShader(fragmentShader);
    if (geometryShader) glDeleteShader(geometryShader);
    if (computeShader) glDeleteShader(computeShader);
}

BE_Shader::~BE_Shader() { glDeleteProgram(ID); }

void BE_Shader::activate() { glUseProgram(ID); }

// ========================================================================

BE_Texture::BE_Texture(const std::string& textureName, const std::string& imageFile, GLuint slot, GLenum type)
    : name(textureName.empty() ? "new texture" : textureName), unit(slot), type(type) {

    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(imageFile.c_str(), &width, &height, &channels, 0);
    if (!data) {
        BE_Message(2, "TEXTURE", "Failed to load texture " + textureName, imageFile.c_str(), 1);
    }

    glGenTextures(1, &ID);
    glActiveTexture(GL_TEXTURE0 + slot);
    unit = slot;
    glBindTexture(type, ID);

    glGenTextures(1, &ID);
    glActiveTexture(GL_TEXTURE0 + slot);
    unit = slot;
    glBindTexture(type, ID);

    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum format = (channels == 4) ? GL_RGBA :
                    (channels == 3) ? GL_RGB :
                    (channels == 1) ? GL_RED : 0;

    if (format == 0) {
        BE_Message(2, "TEXTURE", "Unsupported color channel count " + channels, imageFile.c_str(), 1);
        stbi_image_free(data);
    }

    glTexImage2D(type, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(type);

    stbi_image_free(data);
    glBindTexture(type, 0);
}

BE_Texture::~BE_Texture() { glDeleteTextures(1, &ID); }

void BE_Texture::setUniformUnit(GLuint shaderID, const char* uniform) {
    GLuint loc = glGetUniformLocation(shaderID, uniform);
    glUseProgram(shaderID);
    glUniform1i(loc, unit);
}

void BE_Texture::bind() {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void BE_Texture::unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

// ========================================================================


BE_Camera::BE_Camera(const std::string& cameraName, int width, int height, float fov, float nearPlane, float farPlane, const glm::vec3& pos, const glm::vec3& dir) 
    : name(cameraName.empty() ? "new camera" : cameraName), width(width), height(height), fov(fov), nearPlane(nearPlane), farPlane(farPlane), position(pos) {

    glm::vec3 forward = glm::normalize(dir);
    glm::vec3 defaultForward = glm::vec3(0.0f, 0.0f, -1.0f);
    orientation = glm::rotation(defaultForward, forward);

    zoom = 1.0f;

    projPersp = glm::mat4(1.0f);
    projOrtho = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
}

void BE_Camera::rotate(const glm::vec3& axis, float angle) {
    glm::quat qrot = glm::angleAxis(angle, axis);
    orientation = glm::normalize(qrot * orientation);
}

void BE_Camera::handleInputs(GLFWwindow* window, float dt) {
    float speed = 2.5f;
    float sensitivity = 1.5f;
    glm::vec3 move(0.0f);

    // Direction vectors from orientation
    glm::vec3 forward = orientation * glm::vec3(0,0,-1);
    glm::vec3 right   = orientation * glm::vec3(1,0,0);
    glm::vec3 up      = orientation * glm::vec3(0,1,0);

    glm::vec3 flatForward(forward.x, 0, forward.z);
    if (glm::length(flatForward) > 1e-6f) flatForward = glm::normalize(flatForward);

    glm::vec3 flatRight(right.x, 0, right.z);
    flatRight = glm::normalize(flatRight);

    glm::vec3 flatUp(0, up.y, 0);

    // Movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += flatForward * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= flatForward * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= flatRight * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += flatRight * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) move += flatUp * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) move -= flatUp * speed * dt;

    position += move;

    // Rotation
    float yawDelta = 0.0f, pitchDelta = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT))  yawDelta   = dt * sensitivity;
    if (glfwGetKey(window, GLFW_KEY_RIGHT)) yawDelta   = -dt * sensitivity;
    if (glfwGetKey(window, GLFW_KEY_UP))    pitchDelta = dt * sensitivity;
    if (glfwGetKey(window, GLFW_KEY_DOWN))  pitchDelta = -dt * sensitivity;

    if (std::abs(pitchDelta) > 1e-6f) rotate(glm::vec3(1,0,0), pitchDelta);
    if (std::abs(yawDelta) > 1e-6f)   rotate(glm::vec3(0,1,0), yawDelta);

}

void BE_Camera::updateViewMatrix() {
    glm::vec3 forward = orientation * glm::vec3(0,0,-1);
    glm::vec3 up      = orientation * glm::vec3(0,1,0);
    viewMatrix = glm::lookAt(position, position + forward, up);
}

void BE_Camera::uploadToShader(GLuint shaderID, const char* uniform) {
    glUniform3fv(glGetUniformLocation(shaderID, "camPos"), 1, &position[0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, uniform), 1, GL_FALSE, &projPersp[0][0]);
}

// ========================================================================

// ========================================================================

// ========================================================================

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

BE_Engine::BE_Engine(const std::string& t, int w, int h, const std::source_location& loc) 
    : title(t), width(w), height(h), running(true), window(nullptr) {
    
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
