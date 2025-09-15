#pragma once

#include <BEngine/glad/glad.h>
#include <BEngine/GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <BEngine/glm/glm.hpp>
#include <BEngine/glm/gtc/matrix_transform.hpp>
#include <BEngine/glm/gtc/quaternion.hpp>
#include <BEngine/glm/gtx/quaternion.hpp>
#include <BEngine/glm/gtc/type_ptr.hpp>
#include <BEngine/glm/gtx/euler_angles.hpp>

#include <BEngine/stb/stb_image.h>

#include <string>
#include <source_location>
#include <array>
#include <chrono>
#include <vector>
#include <memory>
#include <unordered_map>

#include "BEngine/engine_default.hpp"

namespace BE {

class FrameTime {
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

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texUV;
};

class VAO {
public:
    GLuint ID = 0;

    VAO();
    ~VAO();
    void bind();
    void unbind();
};

class VBO {
public:
    GLuint ID = 0;

    VBO() = default;
    VBO(GLfloat* vertices, GLsizeiptr size);
    VBO(const std::vector<Vertex>& vertices);
    ~VBO();
    void bind();
    void unbind();
    void linkVertexAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

};

class EBO {
public:
    GLuint ID = 0;

    EBO(GLuint* indices, GLsizeiptr size);
    EBO(const std::vector<GLuint>& indices);
    ~EBO();
    void bind();
    void unbind();
};

class Framebuffer {
public:
    Framebuffer(int width = 1440, int height = 900);
    ~Framebuffer();

    void bind();
    void bindTexture(GLuint shaderID, const char* uniform, int unit);
    void unbind();
    
    GLuint texture = 0;

    void resize(int newWidth, int newHeight, bool linearFilter = false);

private:
    GLuint fbo = 0;
    GLuint rbo = 0;

    int width;
    int height;
    GLenum filter;
    
    void createFramebuffer();
    void destroyFramebuffer();
};

class Shader {
public:
    std::string name;
    GLuint ID = 0;

    Shader(
        const std::string& shaderName, 
        const std::string& vertexPath = "", 
        const std::string& fragmentPath = "", 
        const std::string& geometryPath = "", 
        const std::string& computePath = "",
        const std::string& tessControlPath = "", 
        const std::string& tessEvaluationPath = ""
    );
    
    Shader(
        const std::string& shaderName, 
        const std::string* vertexSource = nullptr, 
        const std::string* fragmentSource = nullptr, 
        const std::string* geometrySource = nullptr, 
        const std::string* computeSource = nullptr,
        const std::string* tessControlSource = nullptr,
        const std::string* tessEvaluationSource = nullptr
    );

    ~Shader();
    void activate();

    void recompile(
        const std::string& vertexPath = "", 
        const std::string& fragmentPath = "", 
        const std::string& geometryPath = "", 
        const std::string& computePath = "",
        const std::string& tessControlPath = "", 
        const std::string& tessEvaluationPath = ""
    );

    void recompile(
        const std::string* vertexSource, 
        const std::string* fragmentSource, 
        const std::string* geometrySource, 
        const std::string* computeSource,
        const std::string* tessControlSource,
        const std::string* tessEvaluationSource
    );
    
private:
    static std::string getFileContents(const std::string& filename);
    static void getCompileErrors(GLuint shader, const std::string& type);
    GLuint compileShader(GLenum type, const std::string& source);
};

class Texture {
public:
    GLuint ID = 0;
    std::string name;
    std::string texType;
    GLuint unit = 0;
    GLenum type = GL_TEXTURE_2D;
    int width = 0;
    int height = 0;
    int channels = 0;

    Texture(const std::string& textureName, const std::string& imagePath, const std::string& texType, GLuint slot);
    Texture(const std::string& textureName, const std::string& texType, int width, int height, const std::string& rawData);
    ~Texture();
    void setUniformUnit(GLuint shaderID, const char* uniform);
    void bind();
    void unbind();
};

class Mesh {
public:
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    
    VAO vao;
    VBO* vbo = nullptr;
    EBO* ebo = nullptr;
    VBO vboInstance;

    std::vector<glm::mat4> instanceModels;

    Mesh() = default;
    Mesh(const std::string& meshName, const std::vector<Vertex>& verts, const std::vector<GLuint>& inds, const std::vector<Texture>& texs);
    Mesh(const std::string& meshName, const std::string& objPath);
    Mesh(const std::string& meshName, const std::string* objSource);
    ~Mesh();

    void addInstance(const glm::vec3& position = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0), const glm::vec3& scale = glm::vec3(1));
    void addInstance(const glm::mat4& model = glm::mat4(1));
    void clearInstances();
    void uploadInstances();

    void draw(Shader& shader, const glm::mat4& modelMatrix);
    void drawInstanced(Shader& shader);

    void loadOBJ(const std::string& objPath);
    void loadOBJSource(const std::string* objSource);

};

class ResourceManager {
public:
    std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

    enum class ShaderType {
        Legacy = 1,
        DSL = 2
    };

    struct ShaderPaths {
        std::string name;
        std::string vertex;
        std::string fragment;
        std::string geometry;
        std::string compute;
        std::string tessControl;
        std::string tessEvaluation;
        std::string dslPath;
        ShaderType type; // 0 = paths, 1 = dsl
    };
    
    std::unordered_map<std::string, ShaderPaths> shaderPaths;

    std::shared_ptr<Mesh> loadMesh(const std::string& name, const std::vector<Vertex>& verts, const std::vector<GLuint>& inds, const std::vector<Texture>& texs, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Mesh> loadMesh(const std::string& name, const std::string& objPath, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Mesh> loadMesh(const std::string& name, const std::string* objSource, const std::source_location& loc = std::source_location::current());
    void removeMesh(const std::string& name, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Mesh> getMesh(const std::string& name, const std::source_location& loc = std::source_location::current());

    std::shared_ptr<Shader> loadShader(
        const std::string& name,
        const std::string& vertexPath = "",
        const std::string& fragmentPath = "",
        const std::string& geometryPath = "",
        const std::string& computePath = "",
        const std::string& tessControlPath = "", 
        const std::string& tessEvaluationPath = "",
        const std::source_location& loc = std::source_location::current()
    );
    std::shared_ptr<Shader> loadShader(
        const std::string& name,
        const std::string* vertexSource = nullptr,
        const std::string* fragmentSource = nullptr,
        const std::string* geometrySource = nullptr,
        const std::string* computeSource = nullptr,
        const std::string* tessControlSource = nullptr,
        const std::string* tessEvaluationSource = nullptr,
        const std::source_location& loc = std::source_location::current()
    );
    void removeShader(const std::string& name, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Shader> getShader(const std::string& name, const std::source_location& loc = std::source_location::current());
    void recompileShaders(const std::source_location& loc = std::source_location::current());

    void loadShaderDSL(const std::string& filePath, const std::source_location& loc = std::source_location::current());

    std::shared_ptr<Texture> loadTexture(const std::string& name, const std::string& imagePath, const std::string& texType, GLuint slot, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Texture> loadTexture(const std::string& name, const std::string& texType, int width, int height, const std::string& rawData, const std::source_location& loc = std::source_location::current());
    void removeTexture(const std::string& name, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Texture> getTexture(const std::string& name, const std::source_location& loc = std::source_location::current());
    
    void loadDefaults();
};

class Camera {
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

    Camera(
        const std::string& cameraName, int width = 1440, int height = 900, 
        float fov = 45.0f, float nearPlane = 0.1f, float farPlane = 100.0f, 
        const glm::vec3& pos = {0,0,0}, const glm::vec3& dir = {0,0,-1} 
    );
    ~Camera() = default;

    // draw(Shader& shader, Mesh& mesh);

    void rotate(const glm::vec3& axis, float angle);
    void handleInputs(GLFWwindow* window, float dt);
    void updateViewMatrix();

    void uploadToShader(GLuint shaderID);
};

class Light {
public:
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 direction;
    glm::mat4 shadowMatrices[2];

    Light(
        float type = 1.0f, 
        const glm::vec3 pos = glm::vec3(0), 
        const glm::vec3 dir = glm::vec3(0), 
        const glm::vec3 col = glm::vec3(1), 
        float inten = 1.0f, float pad1_ = 0.0f
    );
    ~Light() = default;

    void draw(Shader& shader, Mesh& mesh, Camera& camera);

    void generateMatrices();

    void setPosition(const glm::vec3& position);
    void setColor(const glm::vec3& color);
    void setIntensity(float intensity);
    void setDirection(const glm::vec3& direction);
};

class LightManager {
public:
    std::vector<Light> lights;
    std::unordered_map<std::string, size_t> lightLookup;
    // std::vector<Light> activeLights;
    
    size_t maxLights = 64;

    GLuint lightSSBO = 0;
    Light* mappedPtr = nullptr;

    LightManager(size_t maxLights = 128);
    ~LightManager();

    void bind();
    void updateGPU();
    void uploadToShader(GLuint shaderID);
    // void updateActiveLightsForObject(const glm::vec3& objPos, float objRadius);
    void generateAllMatrices();

    size_t addLight(const std::string& name, int type, const std::source_location& loc = std::source_location::current());
    void removeLight(const std::string& name, int type, const std::source_location& loc = std::source_location::current());
    Light* getLight(const std::string& name, const std::source_location& loc = std::source_location::current());
};

class Scene {
public:
    Framebuffer framebuffer;

    std::unordered_map<std::string, std::shared_ptr<Camera>> cameras;
    std::shared_ptr<Camera> activeCamera;
    
    Scene();

    std::shared_ptr<Shader> customShader = nullptr;
    // ResourceManager* resourceManager;

    void setShader(Shader* shader) { setShader(std::shared_ptr<Shader>(shader)); }
    void setShader(std::shared_ptr<Shader> shader) { customShader = shader; }
    void removeShader() { customShader = nullptr; }

    // void setManager(ResourceManager* manager) { resourceManager = manager; }

    void render(ResourceManager& resources, bool renderToFB = false);

    // void render(Shader& shader, Mesh& mesh);

    std::shared_ptr<Camera> addCamera(const std::string& name, const std::source_location& loc = std::source_location::current());
    void removeCamera(const std::string& name, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Camera> getCamera(const std::string& name, const std::source_location& loc = std::source_location::current());

    LightManager& lights() { return lightManager; }
    
private:
    LightManager lightManager;

};

class Engine {
public:
    std::string title;
    int width;
    int height;

    FrameTime frameTime;

    // std::vector<std::shared_ptr<Scene>> scenes;
    // std::shared_ptr<Scene> activeScene;
    // void addScene() {
    //     auto scene = std::make_shared<Scene>();
    //     scenes.push_back(scene);
    //     if (!activeScene) activeScene = scene;
    // }

    /** hello */
    Engine(const std::string& title = "", int width = 1440, int height = 900, const std::source_location& loc = std::source_location::current());
    ~Engine();
    void bind();

    bool isRunning() const;
    void closeWindow();

    void beginFrame();
    void beginRender();

    void setSize(int w, int h) {
        width = w;
        height = h;
        glViewport(0, 0, w, h);
    }

    GLFWwindow* getWindow() { return window; }
    ResourceManager& resources() { return resourceManager; }

private:
    GLFWwindow* window;
    bool running = true;

    ResourceManager resourceManager;
};

} // BE namespace