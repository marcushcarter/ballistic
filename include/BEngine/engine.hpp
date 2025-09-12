#pragma once

#include <BEngine/glad/glad.h>
#include <BEngine/GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <BEngine/glm/glm.hpp>
#include <BEngine/glm/gtc/matrix_transform.hpp>
#include <BEngine/glm/gtc/quaternion.hpp>
#include <BEngine/glm/gtx/quaternion.hpp>
#include <BEngine/glm/gtc/type_ptr.hpp>

#include <BEngine/stb_image/stb_image.h>

#include <string>
#include <source_location>
#include <array>
#include <chrono>
#include <vector>
#include <memory>

#include "BEngine/engine_default.hpp"

class BE_FrameTime {
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
    
private:
    std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point currentTime;

};

struct BE_Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texUV;
};

class BE_VAO {
public:
    GLuint ID = 0;

    BE_VAO();
    ~BE_VAO();
    void bind();
    void unbind();
};

class BE_VBO {
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
public:
    GLuint ID = 0;

    BE_EBO(GLuint* indices, GLsizeiptr size);
    BE_EBO(const std::vector<GLuint>& indices);
    ~BE_EBO();
    void bind();
    void unbind();
};

class BE_Shader {
public:
    std::string name;
    GLuint ID = 0;

    BE_Shader(
        const std::string& shaderName, 
        const std::string& vertexPath = "", 
        const std::string& fragmentPath = "", 
        const std::string& geometryPath = "", 
        const std::string& computePath = ""
    );
    
    BE_Shader(
        const std::string& shaderName, 
        const std::string* vertexSource = nullptr, 
        const std::string* fragmentSource = nullptr, 
        const std::string* geometrySource = nullptr, 
        const std::string* computeSource = nullptr
    );

    ~BE_Shader();
    void activate();

    void recompile(
        const std::string& vertexPath = "", 
        const std::string& fragmentPath = "", 
        const std::string& geometryPath = "", 
        const std::string& computePath = ""
    );
    
private:
    static std::string getFileContents(const std::string& filename);
    static void getCompileErrors(GLuint shader, const std::string& type);
    GLuint compileShader(GLenum type, const std::string& source);
};

class BE_Texture {
public:
    GLuint ID = 0;
    std::string name;
    std::string texType;
    GLuint unit = 0;
    GLenum type = GL_TEXTURE_2D;
    int width = 0;
    int height = 0;
    int channels = 0;

    BE_Texture(const std::string& textureName, const std::string& imagePath, const std::string& texType, GLuint slot);
    BE_Texture(const std::string& textureName, const std::string& texType, int width, int height, const std::string& rawData);
    ~BE_Texture();
    void setUniformUnit(GLuint shaderID, const char* uniform);
    void bind();
    void unbind();
};

class BE_Camera {
public:
    std::string name;
    int width, height;
    float zoom, fov;
    float nearPlane, farPlane;
    float yaw, pitch, roll;
    glm::vec3 position;
    glm::quat orientation;

    glm::mat4 projectionMatrix;
    glm::mat4 orthoMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projViewMatrix;

    BE_Camera(
        const std::string& cameraName, int width = 1440, int height = 900, 
        float fov = 45.0f, float nearPlane = 0.1f, float farPlane = 100.0f, 
        const glm::vec3& pos = {0,0,0}, const glm::vec3& dir = {0,0,-1} 
    );
    ~BE_Camera() = default;

    void rotate(const glm::vec3& axis, float angle);
    void handleInputs(GLFWwindow* window, float dt);
    void updateViewMatrix();

    void uploadToShader(GLuint shaderID);
};

class BE_Mesh {
public:
    std::string name;
    std::vector<BE_Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<BE_Texture> textures;
    
    BE_VAO vao;
    BE_VBO* vbo = nullptr;
    BE_EBO* ebo = nullptr;

    BE_Mesh() = default;
    BE_Mesh(const std::string& meshName, const std::vector<BE_Vertex>& verts, const std::vector<GLuint>& inds, const std::vector<BE_Texture>& texs);
    BE_Mesh(const std::string& meshName, const std::string& objPath);
    BE_Mesh(const std::string& meshName, const std::string* objSource);
    ~BE_Mesh();

    void draw(BE_Shader& shader, const glm::mat4& modelMatrix);
    void loadOBJ(const std::string& objPath);
    void loadOBJSource(const std::string* objSource);
};

class BE_Light {
public:
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 direction;
    glm::mat4 shadowMatrices[2];

    BE_Light(
        float type = 1.0f, 
        const glm::vec3 pos = glm::vec3(0), 
        const glm::vec3 dir = glm::vec3(0,-1,0), 
        const glm::vec3 col = glm::vec3(1), 
        float inten = 1.0f, float pad1_ = 0.0f
    );
    ~BE_Light() = default;
};

class BE_LightManager {
public:
    std::vector<BE_Light> lights;
    std::vector<BE_Light> activeLights;
    
    size_t maxLights = 64;

    GLuint lightSSBO = 0;
    BE_Light* mappedPtr = nullptr;

    BE_LightManager(size_t maxLights = 128);
    ~BE_LightManager();

    void bind();
    void updateGPU();
    void uploadToShader(GLuint shaderID);
    void updateActiveLightsForObject(const glm::vec3& objPos, float objRadius);
    void generateMatrices(BE_Light& light);
    void generateAllMatrices();

    void draw(BE_Shader& shader, BE_Mesh& mesh, BE_Camera& camera);

    void addLight(const BE_Light& light);
    void updateLight(size_t index, const BE_Light& light);
    void removeLight(size_t index);

};

class BE_ResourceManager {
public:
    std::vector<std::shared_ptr<BE_Mesh>> meshes;
    std::shared_ptr<BE_Mesh> loadMesh(const std::string& meshName, const std::vector<BE_Vertex>& verts, const std::vector<GLuint>& inds, const std::vector<BE_Texture>& texs);
    std::shared_ptr<BE_Mesh> loadMesh(const std::string& meshName, const std::string& objPath);
    std::shared_ptr<BE_Mesh> loadMesh(const std::string& meshName, const std::string* objSource);
    std::shared_ptr<BE_Mesh> getMeshPtr(size_t index);

    std::vector<std::shared_ptr<BE_Shader>> shaders;
    std::shared_ptr<BE_Shader> loadShader(
        const std::string& shaderName,
        const std::string& vertexPath = "",
        const std::string& fragmentPath = "",
        const std::string& geometryPath = "",
        const std::string& computePath = ""
    );
    std::shared_ptr<BE_Shader> loadShader(
        const std::string& shaderName,
        const std::string* vertexSource = nullptr,
        const std::string* fragmentSource = nullptr,
        const std::string* geometrySource = nullptr,
        const std::string* computeSource = nullptr
    );
    std::shared_ptr<BE_Shader> getShaderPtr(size_t index);

    std::vector<std::shared_ptr<BE_Texture>> textures;
    std::shared_ptr<BE_Texture> loadTexture(const std::string& textureName, const std::string& imagePath, const std::string& texType, GLuint slot);
    std::shared_ptr<BE_Texture> loadTexture(const std::string& textureName, const std::string& texType, int width, int height, const std::string& rawData);
    std::shared_ptr<BE_Texture> getTexturePtr(size_t index);

    std::unique_ptr<BE_Mesh> cubeMesh;
    std::unique_ptr<BE_Shader> sceneShader;
    std::unique_ptr<BE_Shader> lightShader;
    
    void loadDefaults() {
        cubeMesh = std::make_unique<BE_Mesh>("Cube Mesh", "include/BEngine/meshes/cube.obj");
        sceneShader = std::make_unique<BE_Shader>("Scene Shader", "include/BEngine/shaders/core/sh_core_default.vert", "include/BEngine/shaders/core/sh_core_default.frag");
        lightShader = std::make_unique<BE_Shader>("Light Shader", "include/BEngine/shaders/core/sh_core_default.vert", "include/BEngine/shaders/core/sh_color_uniform.frag");
    }
};

class BE_Scene {
public:
    // std::string name;
    
    BE_LightManager lights;

    // BE_Camera* activeCamera;
    // std::vector<BE_Camera> cameras;

    BE_Scene();
    // ~BE_Scene();

};

class BE_Engine {
public:
    std::string title;

    // std::vector<BE_Scene> scenes;

    /** hello */
    BE_Engine(const std::string& title = "", int width = 1440, int height = 900, const std::source_location& loc = std::source_location::current());
    ~BE_Engine();
    void bind();

    bool isRunning() const;
    void closeWindow();

    void beginFrame();
    void beginRender();
    void endFrame();

    void setSize(int w, int h) {
        width = w;
        height = h;
        glViewport(0, 0, w, h);
    }

    GLFWwindow* getWindow() { return window; }
    BE_FrameTime& getFrameTime() { return frameTime; }
    BE_ResourceManager& resources() { return resourceManager; }

    BE_Camera* activeCamera = nullptr;
    std::unique_ptr<BE_Camera> freeCamera;
    void updateActiveCamera() {
        if (false) {}
        else { activeCamera = freeCamera.get(); }
    }

private:
    GLFWwindow* window;
    bool running = true;
    int width;
    int height;

    BE_FrameTime frameTime;

    BE_ResourceManager resourceManager;
};
