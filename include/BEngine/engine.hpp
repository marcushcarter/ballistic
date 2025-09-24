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
#include <filesystem>
#include <cctype>

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
    int width;
    int height;

    void resize(int newWidth, int newHeight, bool linearFilter = false);

private:
    GLuint fbo = 0;
    GLuint rbo = 0;

    GLenum filter;
    
    void createFramebuffer();
    void destroyFramebuffer();
};

class Shader {
public:
    GLuint ID = 0;

    Shader(
        const std::string& vertexPath = "", 
        const std::string& fragmentPath = "", 
        const std::string& geometryPath = "", 
        const std::string& computePath = "",
        const std::string& tessControlPath = "", 
        const std::string& tessEvaluationPath = ""
    );
    
    Shader(
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
    std::string texType;
    GLenum type = GL_TEXTURE_2D;
    int width = 0;
    int height = 0;
    int channels = 0;

    Texture(const std::string& imagePath, const std::string& texType);
    Texture(const std::string& texType, int width, int height, const std::string& rawData);
    ~Texture();
    void setUniformUnit(GLuint shaderID, const char* uniform, GLuint slot);
    void bind(GLuint slot);
    void unbind();
};

class Material {
public:
    Texture* diffuseMap = nullptr;
    Texture* normalMap = nullptr;
    Texture* roughnessMap = nullptr;

    glm::vec4 diffuseColor = glm::vec4(1,1,1,1);
    float metallic = 0.0f;
    float roughness = 1.0f;

    bool cull = false;
    bool transparent = false;

    Material() = default;
    Material(const std::string mtlPath);

    void uploadToShader(Shader& shader);
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    
    VAO vao;
    VBO* vbo = nullptr;
    EBO* ebo = nullptr;

    Mesh() = default;
    Mesh(const std::vector<Vertex>& verts, const std::vector<GLuint>& inds);
    Mesh(const std::string& objPath);
    Mesh(const std::string* objSource);
    ~Mesh();

    void draw(Shader& shader, const glm::mat4& modelMatrix = glm::mat4(1));
    void makePreview(Framebuffer& fb, Shader& shader, glm::vec2 rotation, bool cull = false);

    void loadOBJ(const std::string& objPath);
    void loadOBJSource(const std::string* objSource);

};

class ResourceManager {
public:
    std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

    enum class ShaderType { Legacy, DSL };

    struct ShaderPaths {
        std::string name;
        std::string vertex;
        std::string fragment;
        std::string geometry;
        std::string compute;
        std::string tessControl;
        std::string tessEvaluation;
        std::string dslPath;
        ShaderType type;
    };
    
    std::unordered_map<std::string, ShaderPaths> shaderPaths;

    std::shared_ptr<Mesh> loadMesh(const std::string& name, const std::vector<Vertex>& verts, const std::vector<GLuint>& inds, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Mesh> loadMesh(const std::string& name, const std::string& objPath, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Mesh> loadMesh(const std::string& name, const std::string* objSource, const std::source_location& loc = std::source_location::current());
    void removeMesh(const std::string& name, const std::source_location& loc = std::source_location::current());

    std::shared_ptr<Material> loadMaterial(const std::string& name, const std::string mtlPath, const std::source_location& loc = std::source_location::current());
    void removeMaterial(const std::string& name, const std::source_location& loc = std::source_location::current());

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
    void recompileShaders(const std::source_location& loc = std::source_location::current());

    void loadShaderDSL(const std::string& filePath, const std::source_location& loc = std::source_location::current());

    std::shared_ptr<Texture> loadTexture(const std::string& name, const std::string& imagePath, const std::string& texType, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Texture> loadTexture(const std::string& name, const std::string& texType, int width, int height, const std::string& rawData, const std::source_location& loc = std::source_location::current());
    void removeTexture(const std::string& name, const std::source_location& loc = std::source_location::current());
    
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
        const glm::vec3& pos = {0,1,1.5}, const glm::vec3& dir = {0,0,-1} 
    );
    ~Camera() = default;

    void rotate(const glm::vec3& axis, float angle);
    void handleInputs(GLFWwindow* window, float dt, bool focusing = false);
    void updateViewMatrix();

    void uploadToShader(GLuint shaderID);
};

class Light {
public:
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 direction;
    glm::mat4 shadowMatrices[2];

    ~Light() = default;
    Light(
        float type = 1.0f, 
        const glm::vec3 pos = glm::vec3(0), 
        const glm::vec3 dir = glm::vec3(0), 
        const glm::vec3 col = glm::vec3(1), 
        float inten = 1.0f, float pad1_ = 0.0f
    );

    void generateMatrices();
};




using Anchor = uint32_t;

enum class AnchorType {
    None, Player
};

struct TagComponent {
    std::string name = "Anchor";
    AnchorType type = AnchorType::None;
};

struct TransformComponent {
    glm::vec3 position {0.0f};
    glm::vec3 rotation {0.0f};
    glm::vec3 scale {1.0f};
};

struct MeshComponent {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    std::shared_ptr<Shader> shader;
};

struct LightComponent {
    glm::vec3 color; 
    float intensity;
    int type;
};

struct Registry {
    std::unordered_map<Anchor, TagComponent> tags;
    std::unordered_map<Anchor, TransformComponent> transforms;
    std::unordered_map<Anchor, MeshComponent> meshes;
    std::unordered_map<Anchor, LightComponent> lights;
};

class Scene {
public:

    // NEW

    std::vector<Anchor> anchors;
    Registry registry;

    Anchor createAnchor();
    void removeAnchor(Anchor a);

    // OLD

    std::unordered_map<std::string, std::unique_ptr<Camera>> cameras;
    Camera* activeCamera;
    
    Scene();

    Camera* addCamera(const std::string& name, const std::source_location& loc = std::source_location::current());
    void removeCamera(const std::string& name, const std::source_location& loc = std::source_location::current());
    
private:
    Anchor nextAnchorID = 0;
};

class Viewport {
public:
    Scene* scene = nullptr;
    Camera* camera = nullptr;
    Framebuffer framebuffer;

    int width = 720;
    int height = 450;

    void resize(int newWidth, int newHeight) {
        width = newWidth;
        height = newHeight;
    }
};

class Engine {
public:
    std::string title;
    int width;
    int height;

    FrameTime frameTime;

    std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
    Scene* activeScene;

    std::unique_ptr<Viewport> viewport;

    Engine(const std::string& title = "", int width = 1440, int height = 900, const std::source_location& loc = std::source_location::current());
    ~Engine();
    void bind();

    Scene* addScene(const std::string& name, const std::source_location& loc = std::source_location::current());
    void removeScene(const std::string& name, const std::source_location& loc = std::source_location::current());

    void renderViewportTexture(Viewport& vp);

    bool isRunning() const;
    void closeWindow();
    void beginFrame();
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



    GLuint lightSSBO = 0;
    Light* mappedPtr = nullptr;
    size_t maxLights = 128;

    Light makeGPULight(const LightComponent& l, const TransformComponent& t) {
        Light gpu;
        gpu.position = glm::vec4(t.position, (float)l.type);
        gpu.color = glm::vec4(l.color, l.intensity);
        return gpu;
    }


};

} // BE namespace