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

namespace Math {

    #define DEG2RAD 0.017453292519943295f
    #define RAD2DEG 57.29577951308232f

    glm::quat EulerToQuat(glm::vec3 euler);

    glm::vec3 QuatToEuler(glm::quat quat);

};

namespace GL {
    
    struct GLState {
        GLuint boundVAO = 0;
        GLuint boundVBO = 0;
        GLuint boundEBO = 0;

        GLuint boundFramebuffer = 0;
        GLuint activatedShader = 0;

        bool cullEnabled = false;
        bool depthEnabled = false;
        bool blendEnabled = false;   
        
    };

    extern GLState g_glState;

    void enableCullFace(bool enable);
    void enableDepthTest(bool enable);
    void enableBlend(bool enable);

}

namespace Stats {
    
    struct RenderStats {

        unsigned int drawCalls;
        unsigned int triangles;
        unsigned int vertices;
        unsigned int indices;

        unsigned int shaderBinds;
        unsigned int textureBinds;
        unsigned int framebufferBinds;

        unsigned int vaoBinds;
        unsigned int vboBinds;
        unsigned int eboBinds;

        void reset() {
            drawCalls = 0;
            triangles = 0;
            vertices = 0;
            indices = 0;

            shaderBinds = 0;
            textureBinds = 0;
            framebufferBinds = 0;

            vaoBinds = 0;
            vboBinds = 0;
            eboBinds = 0;
        }

    };

    extern RenderStats g_renderStats;

};

struct TimeStep {
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

struct VAO {
public:
    GLuint ID = 0;

    VAO();
    ~VAO();
    void bind();
    void unbind();
};

struct VBO {
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

struct EBO {
public:
    GLuint ID = 0;

    EBO(GLuint* indices, GLsizeiptr size);
    EBO(const std::vector<GLuint>& indices);
    ~EBO();
    void bind();
    void unbind();
};

struct AttachmentDesc {
    GLenum attachment;
    GLenum internalFormat;
    GLenum format;
    GLenum type;
    bool isTexture;
};

class Framebuffer {
public:
    GLuint ID = 0;
    int width, height, samples;

    struct Attachment {
        GLuint ID;
        bool isTexture;
        GLenum attachment;
    };

    std::vector<Attachment> attachments;
    std::vector<AttachmentDesc> descriptors;
    
    Framebuffer(int width, int height, const std::vector<AttachmentDesc>& descs, int msaaSamples = 1);

    void bind();
    void unbind();
    void bindTexture(int unit, int index);
    void resize(int newWidth, int newHeight);

    void recreate(const std::vector<AttachmentDesc>& newDescs);

private:
    void destroy();
    void create();
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
    void deactivate();

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
    
    void loadData(const std::vector<Vertex>& verts, const std::vector<GLuint>& inds);
    void loadOBJ(const std::string& objPath);
    void loadOBJ(const std::string* objSource);
    
    ~Mesh();

    void draw(Shader& shader, const glm::mat4& modelMatrix = glm::mat4(1));
    void makePreview(Framebuffer& fb, Shader& shader, glm::vec2 rotation, bool cull = false);

public:
    void parseOBJString(const std::string* objSource, std::vector<Vertex>& outVerts, std::vector<GLuint>& outIndices);
    void updateGPU();
    int GetOrAddVertex(std::vector<Vertex>& vertices, const Vertex& v);
};

struct CreatedShader {
    enum class BaseType { Scene, Post };

    char name[256];
    BaseType type;
    char globalSource[8192];
    char mainSource[8192];
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
    void loadShaderDSL(const std::string* dslSource, const std::source_location& loc = std::source_location::current());

    std::shared_ptr<Texture> loadTexture(const std::string& name, const std::string& imagePath, const std::string& texType, const std::source_location& loc = std::source_location::current());
    std::shared_ptr<Texture> loadTexture(const std::string& name, const std::string& texType, int width, int height, const std::string& rawData, const std::source_location& loc = std::source_location::current());
    void removeTexture(const std::string& name, const std::source_location& loc = std::source_location::current());
    
    void loadDefaults();
};

using Anchor = uint32_t;

enum class AnchorType { None, Player };

struct NameComponent {
    std::string name = "Anchor";
    AnchorType type = AnchorType::None;
};

struct TransformComponent {
    glm::vec3 position {0, 0, 0};
    glm::vec3 rotationEuler {0, 0, 0};
    glm::vec3 scale {1, 1, 1};
    glm::mat4 model {1.0f};
};

struct MeshComponent {
    std::shared_ptr<Mesh> mesh = nullptr;
    std::shared_ptr<Material> material = nullptr;
    std::shared_ptr<Shader> shader = nullptr;
};

struct LightComponent {
    glm::vec3 color {1, 1, 1}; 
    float intensity = 1.0f;
    int type = 1;
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
    Light(
        const TransformComponent& t,
        const LightComponent& l
    );
    ~Light() = default;

    void generateMatrices();
};

struct CameraComponent {
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    float zoom = 1.0f;

    bool isMain = false;
    bool isPerspective = true;
};

struct EditorCamera {
    glm::vec2 orbit = glm::vec2(glm::radians(-45.0f), glm::radians(30.0f));
    float radius = 6.0f;
    glm::vec3 target {0, 0, 0};
    float fov = 45.0f;

    glm::vec2 scrollDelta;
    bool ctrlPressed;
    bool shiftPressed;

    void inputs();
};

class Camera {
public:
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    bool editorCamera;

    Camera() = default;
    Camera(glm::vec3 position, glm::vec3 direction, float fov, float near, float far, bool isPerspective, float aspectRatio);
    Camera(const TransformComponent& t, const CameraComponent& c, float aspectRatio);
    Camera(glm::vec2 orbit, glm::vec3 target, float radius, float near, float far, float aspectRatio);
    Camera(EditorCamera& camera, float near, float far, float aspectRatio);

    void uploadToShader(GLuint shaderID);

};

struct Registry {
    std::unordered_map<Anchor, NameComponent> tags;
    std::unordered_map<Anchor, TransformComponent> transforms;
    std::unordered_map<Anchor, MeshComponent> meshes;
    std::unordered_map<Anchor, LightComponent> lights;
    std::unordered_map<Anchor, CameraComponent> cameras;
};

class Scene {
public:

    std::vector<Anchor> anchors;
    Registry registry;

    Anchor createAnchor();
    void removeAnchor(Anchor a);
    
    Scene();
    
private:
    Anchor nextAnchorID = 0;
};

class Viewport {
public:
    int width, height;
    Framebuffer fbo;
    bool useMSAA;

    Scene* scene = nullptr;
    Camera camera2;

    Viewport(int w, int h, bool msaa = false) : width(w), height(h), useMSAA(msaa), fbo(w, h, buildAttachments(), msaa ? 4 : 1) {}

    void resize(int newW, int newH) {
        if (newW == width && newH == height) return;
        width = newW;
        height = newH;
        fbo.resize(newW, newH);
    }

    void bind() { fbo.bind(); }
    void unbind() { fbo.unbind(); }

    GLuint getColorTexture(int index = 0) { return fbo.attachments[index].ID; }

private:

    std::vector<AttachmentDesc> buildAttachments() {
        // default color + ID + depth tex
        return {
            { GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, true }, // color
            // { GL_COLOR_ATTACHMENT1, GL_RGBA16F, GL_RGBA, GL_FLOAT, true }, // color
            { GL_COLOR_ATTACHMENT1, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, true }, // ID
            { GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, false } // depth
        };
    }

};

class Engine {
public:
    std::string title;
    int width;
    int height;

    TimeStep ts;

    std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
    Scene* activeScene;

    EditorCamera editorCamera;
    std::unique_ptr<Viewport> viewport;

    Engine(const std::string& title = "", int width = 1440, int height = 900, const std::source_location& loc = std::source_location::current());
    ~Engine();

    Scene* addScene(const std::string& name, const std::source_location& loc = std::source_location::current());
    void removeScene(const std::string& name, const std::source_location& loc = std::source_location::current());

    void render();

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