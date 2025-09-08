#include "engine.hpp"
#include "engine_default.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <source_location>
#include <chrono>
#include <cstring>

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
    currentTime = std::chrono::steady_clock::now();
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

// ========================================================================

BE_Texture::BE_Texture(const std::string& textureName, const std::string& imagePath, const std::string& texType, GLuint slot)
    : name(textureName.empty() ? "new texture" : textureName), texType(texType), unit(slot)  {

    type = GL_TEXTURE_2D;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        BE_Message(2, "TEXTURE", "Failed to load texture " + textureName, imagePath.c_str(), 1);
    }

    glGenTextures(1, &ID);
    glActiveTexture(GL_TEXTURE0 + slot);
    unit = slot;
    glBindTexture(type, ID);

    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum format;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_RED;
    else {
        BE_Message(2, "TEXTURE", "Unsupported color channel count " + channels, imagePath.c_str(), 1);
        stbi_image_free(data);
    }

    glTexImage2D(type, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(type);

    stbi_image_free(data);
    glBindTexture(type, 0);
}

BE_Texture::BE_Texture(const std::string& textureName, const std::string& texType, int width, int height, const std::string& rawData)
    : name(textureName), texType(texType) {

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

BE_Texture::~BE_Texture() { glDeleteTextures(1, &ID); }

void BE_Texture::setUniformUnit(GLuint shaderID, const char* uniform) {
    GLuint loc = glGetUniformLocation(shaderID, uniform);
    glUseProgram(shaderID);
    glUniform1i(loc, unit);
}

void BE_Texture::bind() {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(type, ID);
}

void BE_Texture::unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

// ========================================================================

BE_Camera::BE_Camera(const std::string& cameraName, int width, int height, float fov, float nearPlane, float farPlane, const glm::vec3& pos, const glm::vec3& dir) 
    : name(cameraName.empty() ? "new camera" : cameraName), width(width), height(height), fov(fov), nearPlane(nearPlane), farPlane(farPlane), position(pos), zoom(1.0f) {

    glm::vec3 forward = glm::normalize(dir);
    glm::vec3 defaultForward = glm::vec3(0.0f, 0.0f, -1.0f);
    orientation = glm::rotation(defaultForward, forward);

    projectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), nearPlane, farPlane);
    orthoMatrix = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);

    updateViewMatrix();
}

void BE_Camera::rotate(const glm::vec3& axis, float angle) {
    glm::quat qrot = glm::angleAxis(angle, axis);
    orientation = glm::normalize(qrot * orientation);
}

void BE_Camera::handleInputs(GLFWwindow* window, float dt) {
    float speed = 2.5f;
    float sensitivity = 1.5f;

    glm::vec3 move(0.0f);
    glm::vec3 tmp(0.0f);
    glm::vec3 forward, right, up;

    // Rotate standard axes by the camera quaternion
    forward = orientation * glm::vec3(0, 0, -1);
    right   = orientation * glm::vec3(1, 0, 0);
    up      = orientation * glm::vec3(0, 1, 0);

    // Flatten forward/right vectors for FPS movement
    glm::vec3 flatForward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
    glm::vec3 flatRight   = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
    glm::vec3 flatUp      = glm::vec3(0.0f, 1.0f, 0.0f);

    // Movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += flatForward * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= flatForward * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= flatRight * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += flatRight * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) move += flatUp * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) move -= flatUp * speed * dt;

    position += move;

    // Rotation
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  yaw   += dt * sensitivity;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yaw   -= dt * sensitivity;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    pitch += dt * sensitivity;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  pitch -= dt * sensitivity;

    // Clamp pitch to avoid flipping
    if (pitch > glm::radians(89.9f)) pitch = glm::radians(89.9f);
    if (pitch < glm::radians(-89.9f)) pitch = glm::radians(-89.9f);

    // Build quaternions from pitch and yaw
    glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1,0,0));
    glm::quat qYaw   = glm::angleAxis(yaw,   glm::vec3(0,1,0));

    orientation = glm::normalize(qYaw * qPitch);
}

void BE_Camera::updateViewMatrix() {
    glm::vec3 forward = orientation * glm::vec3(0, 0, -1);
    glm::vec3 up      = orientation * glm::vec3(0, 1,  0);
    glm::vec3 target  = position + forward;

    viewMatrix = glm::lookAt(position, target, up);

    projectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), nearPlane, farPlane);

    projViewMatrix = projectionMatrix * viewMatrix;
    
    orthoMatrix = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);

}

void BE_Camera::uploadToShader(GLuint shaderID, const glm::mat4& modelMatrix) {
    glm::mat4 mvp = projViewMatrix * modelMatrix;

    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uMVP"), 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uModel"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uView"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uProjection"), 1, GL_FALSE, &projectionMatrix[0][0]);

    glUniform3fv(glGetUniformLocation(shaderID, "camPos"), 1, &position[0]);
}

// ========================================================================

BE_Mesh::BE_Mesh(const std::string& meshName, const std::vector<BE_Vertex>& verts, const std::vector<GLuint>& inds, const std::vector<BE_Texture>& texs)
    : name(meshName.empty() ? "new mesh" : meshName), vertices(verts), indices(inds), textures(texs) {

    vao.bind();

    vbo = new BE_VBO(vertices);
    ebo = new BE_EBO(indices);

    vbo->linkVertexAttrib(0, 3, GL_FLOAT, sizeof(BE_Vertex), (void*)0);
    vbo->linkVertexAttrib(1, 3, GL_FLOAT, sizeof(BE_Vertex), (void*)(3 * sizeof(float)));
    vbo->linkVertexAttrib(2, 3, GL_FLOAT, sizeof(BE_Vertex), (void*)(6 * sizeof(float)));
    vbo->linkVertexAttrib(3, 2, GL_FLOAT, sizeof(BE_Vertex), (void*)(9 * sizeof(float)));
    
    vao.unbind();
}

BE_Mesh::~BE_Mesh() {
    delete vbo;
    delete ebo;
    
    for (auto& tex : textures) {
        tex.~BE_Texture();
    }
}

void BE_Mesh::draw(BE_Shader& shader) {
    shader.activate();
    vao.bind();

    unsigned int numDiffuse = 0;
    unsigned int numSpecular = 0;

    for (size_t i = 0; i < textures.size(); i++) {
        std::string type = textures[i].texType;
        std::string numStr;

        if (type == "diffuse") { numStr = std::to_string(numDiffuse++); }
        else if (type == "specular") numStr = std::to_string(numSpecular++);

        std::string uniformName = type + numStr;
        textures[i].setUniformUnit(shader.ID, uniformName.c_str());
        textures[i].bind();
    }

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    vao.unbind();

}

int GetOrAddVertex(std::vector<BE_Vertex>& vertices, const BE_Vertex& v) {
    for (size_t i = 0; i < vertices.size(); i++) {
        if (memcmp(&vertices[i], &v, sizeof(BE_Vertex)) == 0) {
            return static_cast<int>(i);
        }
    }
    vertices.push_back(v);
    return static_cast<int>(vertices.size() - 1);
}

void BE_Mesh::loadOBJ(const std::string& objPath) {
    std::ifstream file(objPath);
    if (!file.is_open()) {
        std::cerr << "[Mesh] Failed to open OBJ file: " << objPath << std::endl;
        return;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    std::vector<BE_Vertex> loadedVerts;
    std::vector<GLuint> loadedIndices;

    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;

        if (line.empty() || line[0] == '#' || line.starts_with("o ") || line.starts_with("s "))
            continue;

        std::istringstream ss(line);

        if (line.starts_with("v ")) {
            glm::vec3 v;
            ss.ignore(2);
            ss >> v.x >> v.y >> v.z;
            positions.push_back(v);

        } else if (line.starts_with("vt ")) {
            glm::vec2 vt;
            ss.ignore(3);
            ss >> vt.x >> vt.y;
            uvs.push_back(vt);

        } else if (line.starts_with("vn ")) {
            glm::vec3 vn;
            ss.ignore(3);
            ss >> vn.x >> vn.y >> vn.z;
            normals.push_back(vn);

        } else if (line.starts_with("f ")) {
            ss.ignore(2);

            std::vector<BE_Vertex> faceVerts;
            std::string token;

            while (ss >> token) {
                BE_Vertex vert{};
                int vi = 0, vti = 0, vni = 0;

                if (sscanf(token.c_str(), "%d/%d/%d", &vi, &vti, &vni) == 3) {
                    vi--; vti--; vni--;
                    vert.position = positions[vi];
                    vert.normal   = normals[vni];
                    vert.color    = {1.0f, 1.0f, 1.0f};
                    vert.texUV    = uvs[vti];

                } else if (sscanf(token.c_str(), "%d//%d", &vi, &vni) == 2) {
                    vi--; vni--;
                    vert.position = positions[vi];
                    vert.normal   = normals[vni];
                    vert.color    = {1.0f, 1.0f, 1.0f};
                    vert.texUV    = {0.0f, 0.0f};

                } else if (sscanf(token.c_str(), "%d/%d", &vi, &vti) == 2) {
                    vi--; vti--;
                    vert.position = positions[vi];
                    vert.normal   = {0.0f, 0.0f, 1.0f};
                    vert.color    = {1.0f, 1.0f, 1.0f};
                    vert.texUV    = uvs[vti];

                } else if (sscanf(token.c_str(), "%d", &vi) == 1) {
                    vi--;
                    vert.position = positions[vi];
                    vert.normal   = {0.0f, 0.0f, 1.0f};
                    vert.color    = {1.0f, 1.0f, 1.0f};
                    vert.texUV    = {0.0f, 0.0f};

                } else {
                    std::cerr << "[Mesh] Invalid face token at line " << lineNum << ": " << token << std::endl;
                }

                faceVerts.push_back(vert);
            }

            // triangulate (fan method)
            for (size_t i = 1; i + 1 < faceVerts.size(); i++) {
                int i0 = GetOrAddVertex(loadedVerts, faceVerts[0]);
                int i1 = GetOrAddVertex(loadedVerts, faceVerts[i]);
                int i2 = GetOrAddVertex(loadedVerts, faceVerts[i + 1]);

                loadedIndices.push_back(i1);
                loadedIndices.push_back(i0);
                loadedIndices.push_back(i2);
            }
        }
    }

    file.close();

    this->vertices = std::move(loadedVerts);
    this->indices  = std::move(loadedIndices);
    this->textures = { BE_Texture("fallback", "diffuse", 2, 2, BE::Default::FallbackTexture) };

    vao.bind();
    delete vbo;
    delete ebo;
    vbo = new BE_VBO(this->vertices);
    ebo = new BE_EBO(this->indices);

    vbo->linkVertexAttrib(0, 3, GL_FLOAT, sizeof(BE_Vertex), (void*)0);
    vbo->linkVertexAttrib(1, 3, GL_FLOAT, sizeof(BE_Vertex), (void*)(3 * sizeof(float)));
    vbo->linkVertexAttrib(2, 3, GL_FLOAT, sizeof(BE_Vertex), (void*)(6 * sizeof(float)));
    vbo->linkVertexAttrib(3, 2, GL_FLOAT, sizeof(BE_Vertex), (void*)(9 * sizeof(float)));

    vao.unbind();

    std::cout << "[Mesh] OBJ loaded successfully: " << objPath << std::endl;
}

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

BE_Engine::BE_Engine(const std::string& title, int width, int height, const std::source_location& loc) 
    : title(title.empty() ? "new engine" : title), width(width), height(height), running(true), window(nullptr) {
    
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
    
    // initial scene / resources

    depthShader = std::make_unique<BE_Shader>("DepthShader", &BE::Default::DepthVertexShader);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

    // glCullFace(GL_BACK);
    // glFrontFace(GL_CW);

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void BE_Engine::endFrame() { glfwSwapBuffers(window); }
