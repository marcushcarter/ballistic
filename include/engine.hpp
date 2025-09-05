#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <stb_image/stb_image.h>

#include <string>
#include <source_location>
#include <array>
#include <chrono>
#include <vector>

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

struct BE_Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texUV;
};

class BE_VAO {
private:
public:
    GLuint ID = 0;

    BE_VAO();
    ~BE_VAO();
    void bind();
    void unbind();
};

class BE_VBO {
private:
public:
    GLuint ID = 0;

    BE_VBO(GLfloat* vertices, GLsizeiptr size);
    BE_VBO(const std::vector<BE_Vertex>& vertices);
    ~BE_VBO();
    void bind();
    void unbind();
    void linkVertexAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

};

class BE_EBO {
private:
public:
    GLuint ID = 0;

    BE_EBO(GLuint* indices, GLsizeiptr size);
    BE_EBO(const std::vector<GLuint>& indices);
    ~BE_EBO();
    void bind();
    void unbind();
};

class BE_Shader {
private:
    static std::string getFileContents(const std::string& filename);
    static void getCompileErrors(GLuint shader, const std::string& type);
    GLuint compileShader(GLenum type, const std::string& source);
public:
    std::string name;
    GLuint ID = 0;

    BE_Shader(const std::string& shaderName, 
        const std::string& vertexPath = "", 
        const std::string& fragmentPath = "", 
        const std::string& geometryPath = "", 
        const std::string& computePath = "");
    
    BE_Shader(const std::string& shaderName, 
        const std::string* vertexSource = nullptr, 
        const std::string* fragmentSource = nullptr, 
        const std::string* geometrySource = nullptr, 
        const std::string* computeSource = nullptr);

    ~BE_Shader();
    void activate();
};

class BE_Texture {
private:
public:
    GLuint ID = 0;
    GLuint unit = 0;
    GLenum type = GL_TEXTURE_2D;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::string name;

    BE_Texture() = default;
    BE_Texture(const std::string& textureName, const std::string& imageFile, GLuint slot, GLenum type = GL_TEXTURE_2D);
    // BE_Texture(const std::string& name, int width, int height, unsigned char* data, GLuint slot, GLenum format = GL_RGBA, GLenum type = GL_TEXTURE_2D);
    ~BE_Texture();
    void setUniformUnit(GLuint shaderID, const char* uniform);
    void bind();
    void unbind();

};

class BE_Camera {
private:
public:
    std::string name;
    int width, height;
    float zoom, fov;
    float nearPlane, farPlane;
    glm::vec3 position;
    glm::quat orientation;
    glm::mat4 projPersp, projOrtho;
    glm::mat4 viewMatrix;

    BE_Camera(const std::string& cameraName, int width, int height, float fov, float nearPlane, float farPlane, const glm::vec3& pos, const glm::vec3& dir);
    void rotate(const glm::vec3& axis, float angle);
    void handleInputs(GLFWwindow* window, float dt);
    void updateViewMatrix();
    void uploadToShader(GLuint shaderID, const char* uniform);

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
