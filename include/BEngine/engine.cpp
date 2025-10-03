#include "BEngine/engine.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <source_location>
#include <chrono>
#include <cstring>
#include <stack>

namespace BE {

RenderStats g_renderStats = {0};
   
namespace Math {

glm::quat EulerToQuat(glm::vec3 euler) {
    float yaw = euler.y;
    float pitch = euler.x;
    float roll = euler.z;

    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);
    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);
    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);

    glm::quat q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy + cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy + sr * sp * cy;

    return q;
}

glm::vec3 QuatToEuler(glm::quat quat) {
    glm::vec3 euler;

    float sinr_cosp = 2.0f * (quat.w * quat.x + quat.y * quat.z);
    float cosr_cosp = 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.z);
    euler.x = atan2f(sinr_cosp, cosr_cosp);

    float sinp = 2.0f * (quat.w * quat.y - quat.z * quat.x);
    if (fabsf(sinp) >= 1)
        euler.y = copysignf(90.0f, sinp);
    else 
        euler.y = asinf(sinp);

    float siny_cosp = 2.0f * (quat.w * quat.z + quat.x * quat.y);
    float cosy_cosp = 1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z);
    euler.x = atan2f(siny_cosp, cosy_cosp);
    
    return euler;
}

};

static void Message(int severity, const std::string& module, const std::string& message, const std::string& file = "", int line = 0, const std::source_location& loc = std::source_location::current()) {
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

float TimeStep::update() {
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

VAO::VAO() { glGenVertexArrays(1, &ID); }

VAO::~VAO() { glDeleteVertexArrays(1, &ID); }

static GLuint boundVAO = -1;

void VAO::bind() {
    if (boundVAO != ID) {
        glBindVertexArray(ID);
        boundVAO = ID;
        g_renderStats.vaoBinds++;
    }
}

void VAO::unbind() {
    glBindVertexArray(0);
    boundVAO = -1;

}

// ========================================================================

VBO::VBO(GLfloat* vertices, GLsizeiptr size) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

VBO::VBO(const std::vector<Vertex>& vertices) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

VBO::~VBO() { glDeleteBuffers(1, &ID); }

static GLuint boundVBO = -1;

void VBO::bind() {
    if (boundVBO != ID) {
        glBindBuffer(GL_ARRAY_BUFFER, ID);
        boundVBO = ID;
        g_renderStats.vboBinds++;
    }
}

void VBO::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    boundVBO = -1;
}

void VBO::linkVertexAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    bind();
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    unbind();
}

// ========================================================================

EBO::EBO(GLuint* indices, GLsizeiptr size) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
}

EBO::EBO(const std::vector<GLuint>& indices) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

EBO::~EBO() { glDeleteBuffers(1, &ID); }

static GLuint boundEBO = -1;

void EBO::bind() {
    if (boundEBO != ID) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
        boundEBO = ID;
        g_renderStats.eboBinds++;
    }
}

void EBO::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    boundEBO = -1;
}

// ========================================================================

Framebuffer::Framebuffer(int w, int h, const std::vector<AttachmentDesc>& descs, int msaaSamples) : width(w), height(h), samples(msaaSamples), descriptors(descs) { create(); }

static GLuint boundFramebuffer = -1;

void Framebuffer::bind() {
    if (boundFramebuffer != ID) {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glViewport(0, 0, width, height);

        std::vector<GLenum> drawBuffers;
        for (auto& desc : descriptors) {
                drawBuffers.push_back(desc.type);
            if (desc.type != GL_DEPTH_ATTACHMENT) {
                drawBuffers.push_back(desc.type);
            }
        }

        glDrawBuffers(drawBuffers.size(), drawBuffers.data());
    
        boundFramebuffer = ID;
        g_renderStats.framebufferBinds++;
    }
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    boundFramebuffer = -1;
}

void Framebuffer::bindTexture(int unit, int index) {
    // assert(index < (int)attachments.size());
    if (!attachments[index].isTexture) return;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, attachments[index].ID);
}

void Framebuffer::resize(int newW, int newH) {
    if (newW == width && newH == height) return;

    width = newW;
    height = newH;

    destroy();
    create();
}

void Framebuffer::recreate(const std::vector<AttachmentDesc>& newDescs) {
    descriptors = newDescs;
    create();
}

void Framebuffer::destroy() {
    if (ID) {
        for (auto& a : attachments) {
            if (a.ID) {
                if (a.isTexture) glDeleteTextures(1, &a.ID);
                else glDeleteRenderbuffers(1, &a.ID);
            }
        }
        attachments.clear();
        glDeleteFramebuffers(1, &ID);
        ID = 0;
    }
}

void Framebuffer::create() {
    glGenFramebuffers(1, &ID);
    bind();

    GLenum texTarget = (samples > 1) ? GL_TEXTURE_BINDING_2D_MULTISAMPLE : GL_TEXTURE_2D;
    std::vector<GLenum> drawBuffers;

    for (auto& d : descriptors) {
        Attachment a{};
        a.isTexture = d.isTexture;
        a.attachment = d.attachment;

        if (d.isTexture) {
            glGenTextures(1, &a.ID);
            glBindTexture(texTarget, a.ID);

            if (samples > 1) {
                glTexImage2DMultisample(texTarget, samples, d.internalFormat, width, height, GL_TRUE);
            } else {
                glTexImage2D(texTarget, 0, d.internalFormat, width, height, 0, d.format, d.type, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, d.attachment, texTarget, a.ID, 0);
        } else {
            glGenRenderbuffers(1, &a.ID);
            glBindRenderbuffer(GL_RENDERBUFFER, a.ID);

            if (samples > 1) 
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, d.internalFormat, width, height);
            else
                glRenderbufferStorage(GL_RENDERBUFFER, d.internalFormat, width, height);
            
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, d.attachment, GL_RENDERBUFFER, a.ID);
        }

        attachments.push_back(a);

        if (d.attachment >= GL_COLOR_ATTACHMENT0 && d.attachment <= GL_COLOR_ATTACHMENT31)
            drawBuffers.push_back(d.attachment);
    }

    if (!drawBuffers.empty())
        glDrawBuffers((GLsizei)drawBuffers.size(), drawBuffers.data());
    else 
        glDrawBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer incomplete" << std::endl;
    }

    unbind();
}

// ========================================================================

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const std::string& computePath, const std::string& tessControlPath, const std::string& tessEvaluationPath) {

    GLuint vertexShader = 0, fragmentShader = 0, geometryShader = 0, computeShader = 0, tessControlShader = 0, tessEvaluationShader = 0;

    if (!vertexPath.empty()) vertexShader = compileShader(GL_VERTEX_SHADER, getFileContents(vertexPath));
    if (!fragmentPath.empty()) fragmentShader = compileShader(GL_FRAGMENT_SHADER, getFileContents(fragmentPath));
    if (!geometryPath.empty()) geometryShader = compileShader(GL_GEOMETRY_SHADER, getFileContents(geometryPath));
    if (!computePath.empty()) computeShader = compileShader(GL_COMPUTE_SHADER, getFileContents(computePath));
    if (!tessControlPath.empty()) tessControlShader = compileShader(GL_TESS_CONTROL_SHADER, getFileContents(tessControlPath));
    if (!tessEvaluationPath.empty()) tessEvaluationShader = compileShader(GL_TESS_EVALUATION_SHADER, getFileContents(tessEvaluationPath));

    ID = glCreateProgram();
    if (vertexShader) glAttachShader(ID, vertexShader);
    if (fragmentShader) glAttachShader(ID, fragmentShader);
    if (geometryShader) glAttachShader(ID, geometryShader);
    if (computeShader) glAttachShader(ID, computeShader);
    if (tessControlShader) glAttachShader(ID, tessControlShader);
    if (tessEvaluationShader) glAttachShader(ID, tessEvaluationShader);
    glLinkProgram(ID);
    getCompileErrors(ID, "PROGRAM");

    if (vertexShader) glDeleteShader(vertexShader);
    if (fragmentShader) glDeleteShader(fragmentShader);
    if (geometryShader) glDeleteShader(geometryShader);
    if (computeShader) glDeleteShader(computeShader);
    if (tessControlShader) glDeleteShader(tessControlShader);
    if (tessEvaluationShader) glDeleteShader(tessEvaluationShader);
    
    // Message(0, "SHADER", "Shader '" + shaderName + "' loaded successfully", "PATH", 1);

}

Shader::Shader(const std::string* vertexSource, const std::string* fragmentSource, const std::string* geometrySource, const std::string* computeSource, const std::string* tessControlSource, const std::string* tessEvaluationSource) {

    GLuint vertexShader = 0, fragmentShader = 0, geometryShader = 0, computeShader = 0, tessControlShader = 0, tessEvaluationShader = 0;

    if (vertexSource) vertexShader = compileShader(GL_VERTEX_SHADER, *vertexSource);
    if (fragmentSource) fragmentShader = compileShader(GL_FRAGMENT_SHADER, *fragmentSource);
    if (geometrySource) geometryShader = compileShader(GL_GEOMETRY_SHADER, *geometrySource);
    if (computeSource) computeShader = compileShader(GL_COMPUTE_SHADER, *computeSource);
    if (tessControlSource) tessControlShader = compileShader(GL_TESS_CONTROL_SHADER, *tessControlSource);
    if (tessEvaluationSource) tessEvaluationShader = compileShader(GL_TESS_EVALUATION_SHADER, *tessEvaluationSource);

    ID = glCreateProgram();
    if (vertexShader) glAttachShader(ID, vertexShader);
    if (fragmentShader) glAttachShader(ID, fragmentShader);
    if (geometryShader) glAttachShader(ID, geometryShader);
    if (computeShader) glAttachShader(ID, computeShader);
    if (tessControlShader) glAttachShader(ID, tessControlShader);
    if (tessEvaluationShader) glAttachShader(ID, tessEvaluationShader);
    glLinkProgram(ID);
    getCompileErrors(ID, "PROGRAM");

    if (vertexShader) glDeleteShader(vertexShader);
    if (fragmentShader) glDeleteShader(fragmentShader);
    if (geometryShader) glDeleteShader(geometryShader);
    if (computeShader) glDeleteShader(computeShader);
    if (tessControlShader) glDeleteShader(tessControlShader);
    if (tessEvaluationShader) glDeleteShader(tessEvaluationShader);
    
    // Message(0, "SHADER", "Shader '" + shaderName + "' loaded successfully", "SOURCE", 1);
}

Shader::~Shader() { glDeleteProgram(ID); }

static GLuint boundShader = -1;

void Shader::activate() {
    if (boundShader != ID) {
        glUseProgram(ID); 
        boundShader = ID;
        g_renderStats.shaderBinds++;
    }
}

std::string Shader::getFileContents(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filename << "\n";
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void Shader::getCompileErrors(GLuint shader, const std::string& type) {
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

GLuint Shader::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    switch (type) {
        case GL_VERTEX_SHADER:   getCompileErrors(shader, "VERTEX"); break;
        case GL_FRAGMENT_SHADER: getCompileErrors(shader, "FRAGMENT"); break;
        case GL_GEOMETRY_SHADER: getCompileErrors(shader, "GEOMETRY"); break;
        case GL_COMPUTE_SHADER:  getCompileErrors(shader, "COMPUTE"); break;
        case GL_TESS_CONTROL_SHADER:  getCompileErrors(shader, "TESS CONTROL"); break;
        case GL_TESS_EVALUATION_SHADER:  getCompileErrors(shader, "TESS EVALUATION"); break;
    }
    return shader;
}

void Shader::recompile(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const std::string& computePath, const std::string& tessControlPath, const std::string& tessEvaluationPath) {
    
    GLuint vertexShader = 0, fragmentShader = 0, geometryShader = 0, computeShader = 0, tessControlShader = 0, tessEvaluationShader = 0;

    if (!vertexPath.empty()) vertexShader = compileShader(GL_VERTEX_SHADER, getFileContents(vertexPath));
    if (!fragmentPath.empty()) fragmentShader = compileShader(GL_FRAGMENT_SHADER, getFileContents(fragmentPath));
    if (!geometryPath.empty()) geometryShader = compileShader(GL_GEOMETRY_SHADER, getFileContents(geometryPath));
    if (!computePath.empty()) computeShader = compileShader(GL_COMPUTE_SHADER, getFileContents(computePath));
    if (!tessControlPath.empty()) tessControlShader = compileShader(GL_TESS_CONTROL_SHADER, getFileContents(tessControlPath));
    if (!tessEvaluationPath.empty()) tessEvaluationShader = compileShader(GL_TESS_EVALUATION_SHADER, getFileContents(tessEvaluationPath));

    GLuint newProgram = glCreateProgram();
    if (vertexShader)   glAttachShader(newProgram, vertexShader);
    if (fragmentShader) glAttachShader(newProgram, fragmentShader);
    if (geometryShader) glAttachShader(newProgram, geometryShader);
    if (computeShader)  glAttachShader(newProgram, computeShader);
    if (tessControlShader) glAttachShader(ID, tessControlShader);
    if (tessEvaluationShader) glAttachShader(ID, tessEvaluationShader);
    glLinkProgram(newProgram);
    getCompileErrors(newProgram, "PROGRAM");

    glDeleteProgram(ID);
    ID = newProgram;

    if (vertexShader)   glDeleteShader(vertexShader);
    if (fragmentShader) glDeleteShader(fragmentShader);
    if (geometryShader) glDeleteShader(geometryShader);
    if (computeShader)  glDeleteShader(computeShader);
    if (tessControlShader) glDeleteShader(tessControlShader);
    if (tessEvaluationShader) glDeleteShader(tessEvaluationShader);

}

void Shader::recompile(const std::string* vertexSource, const std::string* fragmentSource, const std::string* geometrySource, const std::string* computeSource, const std::string* tessControlSource, const std::string* tessEvaluationSource) {
    
    GLuint vertexShader = 0, fragmentShader = 0, geometryShader = 0, computeShader = 0, tessControlShader = 0, tessEvaluationShader = 0;

    if (vertexSource) vertexShader = compileShader(GL_VERTEX_SHADER, *vertexSource);
    if (fragmentSource) fragmentShader = compileShader(GL_FRAGMENT_SHADER, *fragmentSource);
    if (geometrySource) geometryShader = compileShader(GL_GEOMETRY_SHADER, *geometrySource);
    if (computeSource) computeShader = compileShader(GL_COMPUTE_SHADER, *computeSource);
    if (tessControlSource) tessControlShader = compileShader(GL_TESS_CONTROL_SHADER, *tessControlSource);
    if (tessEvaluationSource) tessEvaluationShader = compileShader(GL_TESS_EVALUATION_SHADER, *tessEvaluationSource);

    GLuint newProgram = glCreateProgram();
    if (vertexShader)   glAttachShader(newProgram, vertexShader);
    if (fragmentShader) glAttachShader(newProgram, fragmentShader);
    if (geometryShader) glAttachShader(newProgram, geometryShader);
    if (computeShader)  glAttachShader(newProgram, computeShader);
    if (tessControlShader) glAttachShader(ID, tessControlShader);
    if (tessEvaluationShader) glAttachShader(ID, tessEvaluationShader);
    glLinkProgram(newProgram);
    getCompileErrors(newProgram, "PROGRAM");

    glDeleteProgram(ID);
    ID = newProgram;

    if (vertexShader)   glDeleteShader(vertexShader);
    if (fragmentShader) glDeleteShader(fragmentShader);
    if (geometryShader) glDeleteShader(geometryShader);
    if (computeShader)  glDeleteShader(computeShader);
    if (tessControlShader) glDeleteShader(tessControlShader);
    if (tessEvaluationShader) glDeleteShader(tessEvaluationShader);

}

// ========================================================================

Texture::Texture(const std::string& imagePath, const std::string& texType)
    : texType(texType) {

    type = GL_TEXTURE_2D;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        Message(2, "TEXTURE", "Failed to load texture", imagePath.c_str(), 1);
    }

    glGenTextures(1, &ID);
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
        Message(2, "TEXTURE", "Unsupported color channel count " + channels, imagePath.c_str(), 1);
        stbi_image_free(data);
    }

    glTexImage2D(type, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(type);

    stbi_image_free(data);
    glBindTexture(type, 0);
}

Texture::Texture(const std::string& texType, int width, int height, const std::string& rawData)
    : texType(texType) {

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rawData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() { glDeleteTextures(1, &ID); }

void Texture::setUniformUnit(GLuint shaderID, const char* uniform, GLuint slot) {
    GLuint loc = glGetUniformLocation(shaderID, uniform);
    glUniform1i(loc, slot);
}

void Texture::bind(GLuint slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);

    g_renderStats.textureBinds++;
}

void Texture::unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

// ========================================================================

static bool g_cullEnabled = false;

void enableCull(bool enable) {
    if (enable && !g_cullEnabled) {
        glEnable(GL_CULL_FACE);
        g_cullEnabled = true;
    } else if (!enable && g_cullEnabled) {
        glDisable(GL_CULL_FACE);
        g_cullEnabled = false;
    }
}

static bool g_depthEnabled = false;

void enableDepthTest(bool enable) {
    if (enable && !g_depthEnabled) {
        glEnable(GL_DEPTH_TEST);
        g_depthEnabled = true;
    } else if (!enable && g_depthEnabled) {
        glDisable(GL_DEPTH_TEST);
        g_depthEnabled = false;
    }
}

static bool g_blendEnabled = false;

void enableBlend(bool enable) {
    if (enable && !g_blendEnabled) {
        glEnable(GL_BLEND);
        g_blendEnabled = true;
    } else if (!enable && g_blendEnabled) {
        glDisable(GL_BLEND);
        g_blendEnabled = false;
    }
}

// ========================================================================

Material::Material(const std::string mtlPath) {

    diffuseColor = {1,1,1,1};
    metallic = 0.0f;
    roughness = 1.0f;
    cull = true;
    transparent = false;

    
    std::ifstream file(mtlPath);
    if (!file.is_open()) {
        Message(2, "MATERIAL", "Failed to open MTL file " + mtlPath, mtlPath.c_str(), 1);
        return;
    }

    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;

        if (line.empty() || line[0] == '#' || line.starts_with("o ") || line.starts_with("s ")) continue;

        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "Kd") {
            float r, g, b;
            ss >> r >> g >> b;
            diffuseColor = glm::vec4(r, g, b, 1);

        } else if (cmd == "map_Kd") {
            std::string texFile;
            ss >> texFile;
            diffuseMap = new Texture(texFile, "diffuse");

        } else if (cmd == "map_Bump") {
            std::string texFile;
            ss >> texFile;
            normalMap = new Texture(texFile, "normal");

        } else if (cmd == "map_Pr") {
            std::string texFile;
            ss >> texFile;
            roughnessMap = new Texture(texFile, "height");

        } else if (cmd == "d") {
            float opacity;
            ss >> opacity;
            transparent = (opacity < 1.0f);

        } else if (cmd == "Tr") {
            float tr;
            ss >> tr;
            transparent = (tr > 0.0f);

        }
    }

    file.close();
    Message(0, "MATERIAL", "MTL loaded successfully", mtlPath.c_str(), lineNum);

}

void Material::uploadToShader(Shader& shader) {
    shader.activate();

    enableCull(cull);

    // this->diffuseColor;
    // this->metallic;
    // this->roughness;
    // this->transparent;

    glUniform4fv(glGetUniformLocation(shader.ID, "diffuseColor"), 1, glm::value_ptr(diffuseColor));

    if (diffuseMap) {
        diffuseMap->setUniformUnit(shader.ID, "diffuseMap", 0);
        diffuseMap->bind(0);
    } else {
        // backup white color
    }

    if (normalMap) {
        normalMap->setUniformUnit(shader.ID, "normalMap", 1);
        normalMap->bind(1);
    }

    if (roughnessMap) {
        roughnessMap->setUniformUnit(shader.ID, "roughnessMap", 2);
        roughnessMap->bind(2);
    }
}

// ========================================================================

void Mesh::ComputeAABB() {
    if (vertices.empty()) return;

    glm::vec3 min(FLT_MAX);
    glm::vec3 max(-FLT_MAX);

    for (const auto& v : vertices) {
        min.x = std::min(min.x, v.position.x);
        min.y = std::min(min.y, v.position.y);
        min.z = std::min(min.z, v.position.z);

        max.x = std::max(max.x, v.position.x);
        max.y = std::max(max.y, v.position.y);
        max.z = std::max(max.z, v.position.z);
    }

    aabbMin = min;
    aabbMax = max;
}

Mesh::Mesh(const std::vector<Vertex>& verts, const std::vector<GLuint>& inds)
    : vertices(verts), indices(inds) {

    vao.bind();

    vbo = new VBO(vertices);
    ebo = new EBO(indices);

    vbo->linkVertexAttrib(0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    vbo->linkVertexAttrib(1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    vbo->linkVertexAttrib(2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    vbo->linkVertexAttrib(3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));

    ComputeAABB();
}

Mesh::Mesh(const std::string& objPath)
    : vertices(), indices() {
    loadOBJ(objPath.c_str());
}

Mesh::Mesh(const std::string* objSource)
    : vertices(), indices() {
    loadOBJSource(objSource);
}

Mesh::~Mesh() {
    delete vbo;
    delete ebo;
}

void Mesh::draw(Shader& shader, const glm::mat4& modelMatrix) {
    shader.activate();
    vao.bind();

    enableDepthTest(true);

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "uModel"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    g_renderStats.drawCalls++;
    g_renderStats.vertices += vertices.size();
    g_renderStats.indices += indices.size();
    g_renderStats.triangles += indices.size()/3;
}

void Mesh::makePreview(Framebuffer& fb, Shader& shader, glm::vec2 rotation, bool cull) {

    enableCull(cull);

    fb.bind();

    glClearColor(0.05,0.05,0.05,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec3 minBounds(FLT_MAX);
    glm::vec3 maxBounds(-FLT_MAX);

    for (auto& v : vertices) {
        minBounds = glm::min(minBounds, v.position);
        maxBounds = glm::max(maxBounds, v.position);
    }

    glm::vec3 center = (minBounds + maxBounds) * 0.5f;
    glm::vec3 extents = maxBounds - minBounds;
    float maxExtent = glm::compMax(extents);

    const float padding = 1.2f;
    float scaleFactor = 2.0f / (maxExtent * padding);

    glm::vec3 cameraPos(0,0,1);

    glm::vec3 cameraForward = glm::normalize(cameraPos - center);
    glm::vec3 cameraRight = glm::cross(glm::vec3(0,1,0), cameraForward);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(1,0,0));
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(0,1,0));
    model = glm::translate(model, -center);

    glm::mat4 view = glm::lookAt(cameraPos * (maxExtent * 2.0f), glm::vec3(0), glm::vec3(0,1,0));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, maxExtent * 10.0f);

    shader.activate();

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "uView"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "uProjection"), 1, GL_FALSE, glm::value_ptr(proj));

    shader.activate();
    vao.bind();

    enableDepthTest(true);

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    fb.unbind();
}

int GetOrAddVertex(std::vector<Vertex>& vertices, const Vertex& v) {
    for (size_t i = 0; i < vertices.size(); i++) {
        if (memcmp(&vertices[i], &v, sizeof(Vertex)) == 0) {
            return static_cast<int>(i);
        }
    }
    vertices.push_back(v);
    return static_cast<int>(vertices.size() - 1);
}

void Mesh::loadOBJ(const std::string& objPath) {
    std::ifstream file(objPath);
    if (!file.is_open()) {
        Message(2, "MESH", "Failed to open OBJ file " + objPath, objPath.c_str(), 1);
        return;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    std::vector<Vertex> loadedVerts;
    std::vector<GLuint> loadedIndices;

    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;

        if (line.empty() || line[0] == '#' || line.starts_with("o ") || line.starts_with("s ")) continue;

        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (line.starts_with("v ")) {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            positions.push_back(v);

        } else if (line.starts_with("vt ")) {
            glm::vec2 vt;
            ss >> vt.x >> vt.y;
            uvs.push_back(vt);

        } else if (line.starts_with("vn ")) {
            glm::vec3 vn;
            ss >> vn.x >> vn.y >> vn.z;
            normals.push_back(vn);

        } else if (line.starts_with("f ")) {

            std::vector<Vertex> faceVerts;
            std::string token;

            while (ss >> token) {
                Vertex vert{};
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
                    Message(1, "MESH", "Invalid face token: " + token, objPath.c_str(), lineNum);
                }

                faceVerts.push_back(vert);
            }

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

    vao.bind();
    delete vbo;
    delete ebo;
    vbo = new VBO(this->vertices);
    ebo = new EBO(this->indices);

    vbo->linkVertexAttrib(0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    vbo->linkVertexAttrib(1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    vbo->linkVertexAttrib(2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    vbo->linkVertexAttrib(3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));

    ComputeAABB();

    Message(0, "MESH", "OBJ loaded successfully", objPath.c_str(), lineNum);
}

void Mesh::loadOBJSource(const std::string* objSource) {
    if (!objSource) {
        Message(2, "MESH", "Could not find OBJ source", "SOURCE", 1);
        return;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    std::vector<Vertex> loadedVerts;
    std::vector<GLuint> loadedIndices;

    std::istringstream file(*objSource);
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;

        if (line.empty() || line[0] == '#' || line.starts_with("o ") || line.starts_with("s "))
            continue;

        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (line.starts_with("v ")) {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            positions.push_back(v);

        } else if (line.starts_with("vt ")) {
            glm::vec2 vt;
            ss >> vt.x >> vt.y;
            uvs.push_back(vt);

        } else if (line.starts_with("vn ")) {
            glm::vec3 vn;
            ss >> vn.x >> vn.y >> vn.z;
            normals.push_back(vn);

        } else if (line.starts_with("f ")) {

            std::vector<Vertex> faceVerts;
            std::string token;

            while (ss >> token) {
                Vertex vert{};
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
                    Message(1, "MESH", "Invalid face token: " + token, "SOURCE", lineNum);
                }

                faceVerts.push_back(vert);
            }

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

    this->vertices = std::move(loadedVerts);
    this->indices  = std::move(loadedIndices);

    vao.bind();
    delete vbo;
    delete ebo;
    vbo = new VBO(this->vertices);
    ebo = new EBO(this->indices);

    vbo->linkVertexAttrib(0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    vbo->linkVertexAttrib(1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    vbo->linkVertexAttrib(2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    vbo->linkVertexAttrib(3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));

    ComputeAABB();

    Message(0, "MESH", "OBJ loaded successfully", "SOURCE", lineNum);
}

// ========================================================================

std::shared_ptr<Mesh> ResourceManager::loadMesh(const std::string& name, const std::vector<Vertex>& verts, const std::vector<GLuint>& inds, const std::source_location& loc) {
    auto it = meshes.find(name);
    if (it != meshes.end()) {
        Message(1, "RESOURCE", "Mesh '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second;
    }
    
    auto mesh = std::make_shared<Mesh>(verts, inds);
    meshes[name] = mesh;
    return mesh;
}

std::shared_ptr<Mesh> ResourceManager::loadMesh(const std::string& name, const std::string& objPath, const std::source_location& loc) {
    auto it = meshes.find(name);
    if (it != meshes.end()) {
        Message(1, "RESOURCE", "Mesh '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second;
    }
    
    auto mesh = std::make_shared<Mesh>(objPath);
    meshes[name] = mesh;
    return mesh;
}

std::shared_ptr<Mesh> ResourceManager::loadMesh(const std::string& name, const std::string* objSource, const std::source_location& loc) {
    auto it = meshes.find(name);
    if (it != meshes.end()) {
        Message(1, "RESOURCE", "Mesh '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second;
    }
    
    auto mesh = std::make_shared<Mesh>(objSource);
    meshes[name] = mesh;
    return mesh;
}

void ResourceManager::removeMesh(const std::string& name, const std::source_location& loc) {
    auto it = meshes.find(name);
    if (it != meshes.end()) {
        meshes.erase(it);
    } else {
        Message(2, "RESOURCE", "Could not find mesh '" + name + "'", loc.file_name(), loc.line());
    }    
}

std::shared_ptr<Material> ResourceManager::loadMaterial(const std::string& name, const std::string mtlPath, const std::source_location& loc) {
    auto it = materials.find(name);
    if (it != materials.end()) {
        Message(1, "RESOURCE", "Material '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second;
    }
    
    auto material = std::make_shared<Material>(mtlPath);
    materials[name] = material;
    return material;
}

void ResourceManager::removeMaterial(const std::string& name, const std::source_location& loc) {
    auto it = materials.find(name);
    if (it != materials.end()) {
        materials.erase(it);
    } else {
        Message(2, "RESOURCE", "Could not find Material '" + name + "'", loc.file_name(), loc.line());
    }    
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const std::string& computePath, const std::string& tessControlPath, const std::string& tessEvaluationPath, const std::source_location& loc) {
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        Message(1, "RESOURCE", "Shader '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second;
    }

    ShaderPaths sp;
    sp.name = name;
    sp.type = ShaderType::Legacy;
    sp.vertex = vertexPath;
    sp.fragment = fragmentPath;
    sp.geometry = geometryPath;
    sp.compute = computePath;
    sp.tessControl = tessControlPath;
    sp.tessEvaluation = tessEvaluationPath;
    shaderPaths[name] = sp;
    
    auto shader = std::make_shared<Shader>(vertexPath, fragmentPath, geometryPath, computePath, tessControlPath, tessEvaluationPath);
    shaders[name] = shader;
    
    // Message(0, "RESOURCE", "Shader '" + name + "' loaded successfully from PATHS", loc.file_name(), loc.line());

    return shader;
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string& name, const std::string* vertexSource, const std::string* fragmentSource, const std::string* geometrySource, const std::string* computeSource, const std::string* tessControlSource, const std::string* tessEvaluationSource, const std::source_location& loc) {
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        Message(1, "RESOURCE", "Shader '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second;
    }
    
    auto shader = std::make_shared<Shader>(vertexSource, fragmentSource, geometrySource, computeSource, tessControlSource, tessEvaluationSource);
    shaders[name] = shader;
    
    // Message(0, "RESOURCE", "Shader '" + name + "' loaded successfully from STRINGS", loc.file_name(), loc.line());

    return shader;
}

const char* GLTypeToString(GLenum type) {
    switch (type) {
        case GL_VERTEX_SHADER: return "GL_VERTEX_SHADER";
        case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
        case GL_GEOMETRY_SHADER: return "GL_GEOMETRY_SHADER";
        case GL_COMPUTE_SHADER: return "GL_COMPUTE_SHADER";
        case GL_TESS_CONTROL_SHADER: return "GL_TESS_CONTROL_SHADER";
        case GL_TESS_EVALUATION_SHADER: return "GL_TESS_EVALUATION_SHADER";
        default: return "UNKNOWN_GLENUM";
    }
}

std::string join_names(const std::vector<std::string>& names) {
    if (names.empty()) return "";

    std::ostringstream oss;
    for (size_t i = 0; i < names.size(); i++) {
        oss << "'" << names[i] << "'";
        if (i < names.size() - 2) {
            oss << " ";
        } else if (i == names.size() - 2) {
            oss << " and ";
        }
    }
    return oss.str();
}

void ResourceManager::loadShaderDSL(const std::string& filePath, const std::source_location& loc) {
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        Message(2, "RESOURCE", "Failed to open DSL file " + filePath, loc.file_name(), loc.line());
        return;
    }

    struct ShaderSource {
        std::string source;
        GLenum type;
    };

    std::unordered_map<std::string, ShaderSource> localShaders;
    std::vector<std::string> shaderNames;

    ShaderSource currentShader;
    std::string currentName;
    bool writingShader = false;

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        lineNum++;

        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty()) continue;

        if (line[0] == '@') {
            if (writingShader) {
                localShaders[currentName] = currentShader;
                currentShader = {};
                currentName.clear();
                writingShader = false;
            }

            if (line.find("@vs") == 0) {
                currentName = line.substr(4);
                currentShader.type = GL_VERTEX_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@fs") == 0) {
                currentName = line.substr(4);
                currentShader.type = GL_FRAGMENT_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@gs") == 0) {
                currentName = line.substr(4);
                currentShader.type = GL_GEOMETRY_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@cs") == 0) {
                currentName = line.substr(4);
                currentShader.type = GL_COMPUTE_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@tcs") == 0) {
                currentName = line.substr(5);
                currentShader.type = GL_TESS_CONTROL_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@tes") == 0) {
                currentName = line.substr(5);
                currentShader.type = GL_TESS_EVALUATION_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@program") == 0) {
                std::istringstream ss(line.substr(9));
                std::string programName;
                ss >> programName;

                std::string vertexName, fragmentName, geometryName, computeName, tessControlName, tessEvaluationName;

                std::unordered_map<GLenum, std::string> usedTypes;

                std::string shaderName;
                while (ss >> shaderName) {
                    auto it = localShaders.find(shaderName);
                    if (it == localShaders.end()) {
                        Message(2, "SHADER", "Shader '" + shaderName + "' not found in DSL file", filePath.c_str(), lineNum);
                        continue;
                    }

                    GLenum type = it->second.type;

                    if (usedTypes.find(type) != usedTypes.end()) {
                        Message(1, "SHADER", "Program '" + programName + "' already has a shader of type " + GLTypeToString(type) + " (" + usedTypes[type] + ")", filePath.c_str(), lineNum);
                        continue;
                    }

                    usedTypes[type] = shaderName;

                    if (type == GL_VERTEX_SHADER) vertexName = shaderName;
                    else if (type == GL_FRAGMENT_SHADER) fragmentName = shaderName;
                    else if (type == GL_GEOMETRY_SHADER) geometryName = shaderName;
                    else if (type == GL_COMPUTE_SHADER) computeName = shaderName;
                    else if (type == GL_TESS_CONTROL_SHADER) tessControlName = shaderName;
                    else if (type == GL_TESS_EVALUATION_SHADER) tessEvaluationName = shaderName;

                }

                const std::string* vsSrc = vertexName.empty() ? nullptr : &localShaders[vertexName].source;
                const std::string* fsSrc = fragmentName.empty() ? nullptr : &localShaders[fragmentName].source;
                const std::string* gsSrc = geometryName.empty() ? nullptr : &localShaders[geometryName].source;
                const std::string* csSrc = computeName.empty() ? nullptr : &localShaders[computeName].source;
                const std::string* tcsSrc = tessControlName.empty() ? nullptr : &localShaders[tessControlName].source;
                const std::string* tesSrc = tessEvaluationName.empty() ? nullptr : &localShaders[tessEvaluationName].source;

                auto it = shaders.find(programName);
                if (it != shaders.end()) {
                    shaders[programName]->recompile(vsSrc, fsSrc, gsSrc, csSrc, tcsSrc, tesSrc);
                } else {
                    loadShader(programName, vsSrc, fsSrc, gsSrc, csSrc, tcsSrc, tesSrc, loc);
                }

                shaderNames.push_back(programName);
            }

        } else if (writingShader) {
            currentShader.source += line + "\n";
        }
    }

    if (writingShader) localShaders[currentName] = currentShader;

    file.close();

    ShaderPaths sp;
    sp.type = ShaderType::DSL;
    sp.dslPath = filePath;
    shaderPaths[filePath] = sp;

    std::string namesStr = join_names(shaderNames);

    Message(0, "RESOURCE", "DSL shaders " + namesStr + "loaded successfully", filePath.c_str(), lineNum);
}

void ResourceManager::loadShaderDSL(const std::string* dslSource, const std::source_location& loc) {
    if (!dslSource) {
        Message(2, "SHADER", "Could not find DSL source", "SOURCE", 1);
        return;
    }
    std::istringstream file(*dslSource);

    struct ShaderSource {
        std::string source;
        GLenum type;
    };

    std::unordered_map<std::string, ShaderSource> localShaders;
    std::vector<std::string> shaderNames;

    ShaderSource currentShader;
    std::string currentName;
    bool writingShader = false;

    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;

        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty()) continue;

        if (line[0] == '@') {
            if (writingShader) {
                localShaders[currentName] = currentShader;
                currentShader = {};
                currentName.clear();
                writingShader = false;
            }

            if (line.find("@vs") == 0) {
                currentName = line.substr(4);
                currentShader.type = GL_VERTEX_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@fs") == 0) {
                currentName = line.substr(4);
                currentShader.type = GL_FRAGMENT_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@gs") == 0) {
                currentName = line.substr(4);
                currentShader.type = GL_GEOMETRY_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@cs") == 0) {
                currentName = line.substr(4);
                currentShader.type = GL_COMPUTE_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@tcs") == 0) {
                currentName = line.substr(5);
                currentShader.type = GL_TESS_CONTROL_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@tes") == 0) {
                currentName = line.substr(5);
                currentShader.type = GL_TESS_EVALUATION_SHADER;
                currentShader.source.clear();
                writingShader = true;
            } else if (line.find("@program") == 0) {
                std::istringstream ss(line.substr(9));
                std::string programName;
                ss >> programName;

                std::string vertexName, fragmentName, geometryName, computeName, tessControlName, tessEvaluationName;

                std::unordered_map<GLenum, std::string> usedTypes;

                std::string shaderName;
                while (ss >> shaderName) {
                    auto it = localShaders.find(shaderName);
                    if (it == localShaders.end()) {
                        Message(2, "SHADER", "Shader '" + shaderName + "' not found in DSL file", "SOURCE", lineNum);
                        continue;
                    }

                    GLenum type = it->second.type;

                    if (usedTypes.find(type) != usedTypes.end()) {
                        Message(1, "SHADER", "Program '" + programName + "' already has a shader of type " + GLTypeToString(type) + " (" + usedTypes[type] + ")", "SOURCE", lineNum);
                        continue;
                    }

                    usedTypes[type] = shaderName;

                    if (type == GL_VERTEX_SHADER) vertexName = shaderName;
                    else if (type == GL_FRAGMENT_SHADER) fragmentName = shaderName;
                    else if (type == GL_GEOMETRY_SHADER) geometryName = shaderName;
                    else if (type == GL_COMPUTE_SHADER) computeName = shaderName;
                    else if (type == GL_TESS_CONTROL_SHADER) tessControlName = shaderName;
                    else if (type == GL_TESS_EVALUATION_SHADER) tessEvaluationName = shaderName;

                }

                const std::string* vsSrc = vertexName.empty() ? nullptr : &localShaders[vertexName].source;
                const std::string* fsSrc = fragmentName.empty() ? nullptr : &localShaders[fragmentName].source;
                const std::string* gsSrc = geometryName.empty() ? nullptr : &localShaders[geometryName].source;
                const std::string* csSrc = computeName.empty() ? nullptr : &localShaders[computeName].source;
                const std::string* tcsSrc = tessControlName.empty() ? nullptr : &localShaders[tessControlName].source;
                const std::string* tesSrc = tessEvaluationName.empty() ? nullptr : &localShaders[tessEvaluationName].source;

                auto it = shaders.find(programName);
                if (it != shaders.end()) {
                    shaders[programName]->recompile(vsSrc, fsSrc, gsSrc, csSrc, tcsSrc, tesSrc);
                } else {
                    loadShader(programName, vsSrc, fsSrc, gsSrc, csSrc, tcsSrc, tesSrc, loc);
                }

                shaderNames.push_back(programName);
            }

        } else if (writingShader) {
            currentShader.source += line + "\n";
        }
    }

    if (writingShader) localShaders[currentName] = currentShader;

    // ShaderPaths sp;
    // sp.type = ShaderType::DSL;
    // sp.dslPath = filePath;
    // shaderPaths[filePath] = sp;

    std::string namesStr = join_names(shaderNames);

    Message(0, "RESOURCE", "DSL shaders " + namesStr + "loaded successfully", "SOURCE", lineNum);
}

void ResourceManager::removeShader(const std::string& name, const std::source_location& loc) {
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        shaders.erase(it);
    } else {
        Message(2, "RESOURCE", "Could not find shader '" + name + "'", loc.file_name(), loc.line());
    }    
}

void ResourceManager::recompileShaders(const std::source_location& loc) {
    for (const auto& [key, shader] : shaderPaths) {
        if (shader.type == ShaderType::Legacy) {
            if (shaders.contains(shader.name)) shaders[shader.name]->recompile(shader.vertex, shader.fragment, shader.geometry, shader.compute, shader.tessControl, shader.tessEvaluation);
        } else if (shader.type == ShaderType::DSL) {
            loadShaderDSL(shader.dslPath, loc);
        }
    }
}

std::shared_ptr<Texture> ResourceManager::loadTexture(const std::string& name, const std::string& imagePath, const std::string& texType, const std::source_location& loc) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        Message(1, "RESOURCE", "Texture '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second;
    }
    
    auto texture = std::make_shared<Texture>(imagePath, texType);
    textures[name] = texture;
    return texture;
}

std::shared_ptr<Texture> ResourceManager::loadTexture(const std::string& name, const std::string& texType, int width, int height, const std::string& rawData, const std::source_location& loc) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        Message(1, "RESOURCE", "Texture '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second;
    }
    
    auto texture = std::make_shared<Texture>(texType, width, height, rawData);
    textures[name] = texture;
    return texture;
}

void ResourceManager::removeTexture(const std::string& name, const std::source_location& loc) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        textures.erase(it);
    } else {
        Message(2, "RESOURCE", "Could not find texture '" + name + "'", loc.file_name(), loc.line());
    }    
}

void ResourceManager::loadDefaults() {
    loadMesh("default_cube", &BE::Default::cube_obj);
    loadMesh("default_quad", &BE::Default::quad_obj);
    loadShaderDSL(&BE::Default::defaults_dsl);

    auto texture = std::make_shared<Texture>("diffuse", 4, 4, BE::Default::FallbackTexture);
    textures["default_texture"] = texture;

    materials["default_material"] = std::make_shared<Material>();
    materials["default_material"]->diffuseMap = textures["default_texture"].get();
}

// ========================================================================

Light::Light(float type, const glm::vec3 pos, const glm::vec3 dir, const glm::vec3 col, float inten, float pad1_ ) {
    
    position = glm::vec4(pos, (float)type);
    color = glm::vec4(col, inten);
    direction = glm::vec4(dir, pad1_);
    generateMatrices();
}

Light::Light(const TransformComponent& t, const LightComponent& l) {

    position = glm::vec4(t.position, (float)l.type);
    color = glm::vec4(l.color, l.intensity);
    direction = glm::vec4(t.rotationEuler, 0.0f);
    generateMatrices();
}

void Light::generateMatrices() {
    glm::vec3 pos = glm::vec3(position);
    glm::vec3 dir = glm::vec3(direction);

    if (position.w == 0.0f) {
        float orthoSize = 20.0f;
        glm::mat4 proj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(pos, pos + dir, glm::vec3(0, 1, 0));
        shadowMatrices[0] = proj * view;
    }

    else if (position.w == 1.0f) {
        float fov    = glm::radians(180.0f);
        float aspect = 1.0f;
        float range  = direction.w;
        glm::mat4 proj = glm::perspective(fov, aspect, 0.1f, range);

        shadowMatrices[0] = proj * glm::lookAt(pos, pos + glm::vec3(0, 0,  1), glm::vec3(0, 1, 0));
        shadowMatrices[1] = proj * glm::lookAt(pos, pos + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    }

    else if (position.w == 2.0f) {
        float fov    = glm::radians(45.0f);
        float aspect = 1.0f;
        float range  = direction.w;
        glm::mat4 proj = glm::perspective(fov, aspect, 0.1f, range);
        glm::mat4 view = glm::lookAt(pos, pos + dir, glm::vec3(0, 1, 0));
        shadowMatrices[0] = proj * view;
    }


}

// ========================================================================

Camera::Camera(const TransformComponent& t, const CameraComponent& c, float aspectRatio) : editorCamera(false) {   

    if (c.isPerspective) {
        projectionMatrix = glm::perspective(
            glm::radians(c.fov),
            aspectRatio,
            c.nearPlane,
            c.farPlane
        );
    } else {
        float orthoHeight = 10.0f;
        float orthoWidth = orthoHeight * aspectRatio;
        projectionMatrix = glm::ortho(
            -orthoWidth * 0.5f, orthoWidth * 0.5f,
            -orthoHeight * 0.5f, orthoHeight * 0.5f,
            c.nearPlane, c.farPlane
        );
    }

    glm::quat orientation = glm::quat(t.rotationEuler);

    glm::vec3 forward = orientation * glm::vec3(0, 0, -1);
    glm::vec3 up      = orientation * glm::vec3(0, 1,  0);
    glm::vec3 target  = t.position + forward;

    viewMatrix = glm::lookAt(t.position, target, up);
}

Camera::Camera(glm::vec3 position, glm::vec3 direction, float fov, float near, float far, bool isPerspective, float aspectRatio) : editorCamera(false) {

    if (isPerspective) {
        projectionMatrix = glm::perspective(
            glm::radians(fov),
            aspectRatio,
            near,
            far
        );
    } else {
        float orthoHeight = 10.0f;
        float orthoWidth = orthoHeight * aspectRatio;
        projectionMatrix = glm::ortho(
            -orthoWidth * 0.5f, orthoWidth * 0.5f,
            -orthoHeight * 0.5f, orthoHeight * 0.5f,
            near, far
        );
    }

    glm::quat orientation = glm::quat(direction);

    glm::vec3 forward = orientation * glm::vec3(0, 0, -1);
    glm::vec3 up      = orientation * glm::vec3(0, 1,  0);
    glm::vec3 target  = position + forward;

    viewMatrix = glm::lookAt(position, target, up);
}

Camera::Camera(glm::vec2 orbit, glm::vec3 target, float radius, float near, float far, float aspectRatio) : editorCamera(true) {
    
    projectionMatrix = glm::perspective(
        glm::radians(60.0f),
        aspectRatio,
        near,
        far
    );
    
    glm::vec3 position = target + glm::vec3(
        radius * cosf(glm::clamp(orbit.y, -glm::radians(89.0f), glm::radians(89.0f))) * cosf(orbit.x),
        radius * sinf(glm::clamp(orbit.y, -glm::radians(89.0f), glm::radians(89.0f))),
        radius * cosf(glm::clamp(orbit.y, -glm::radians(89.0f), glm::radians(89.0f))) * sinf(orbit.x)
    );

    viewMatrix = glm::lookAt(position, target, glm::vec3(0,1,0));
}

Camera::Camera(EditorCamera& camera, float near, float far, float aspectRatio) : editorCamera(true) {
    
    projectionMatrix = glm::perspective(glm::radians(camera.fov), aspectRatio, near, far);

    glm::vec3 position = camera.target + glm::vec3(
        camera.radius * cosf(glm::clamp(camera.orbit.y, -glm::radians(89.0f), glm::radians(89.0f))) * cosf(camera.orbit.x),
        camera.radius * sinf(glm::clamp(camera.orbit.y, -glm::radians(89.0f), glm::radians(89.0f))),
        camera.radius * cosf(glm::clamp(camera.orbit.y, -glm::radians(89.0f), glm::radians(89.0f))) * sinf(camera.orbit.x)
    );

    viewMatrix = glm::lookAt(position, camera.target, glm::vec3(0,1,0));
}

void Camera::uploadToShader(GLuint shaderID) {
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uView"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uProjection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
}

void EditorCamera::inputs() {

    float orbitSpeed = 0.01f;
    float zoomSpeed = 0.01f;
    float panSpeed = 0.01f * radius;

    if (!ctrlPressed && !shiftPressed) {

        orbit.x += scrollDelta.x * orbitSpeed;
        orbit.y += scrollDelta.y * orbitSpeed;
        orbit.y = glm::clamp(orbit.y, -glm::radians(89.0f), glm::radians(89.0f));

    } else if (ctrlPressed) {

        radius -= scrollDelta.x * zoomSpeed;
        radius += scrollDelta.y * zoomSpeed;
        radius = glm::max(radius, 0.1f);

    } else if (shiftPressed) {
        
        glm::vec3 cameraPos = target + glm::vec3(
            radius * cosf(glm::clamp(orbit.y, -glm::radians(89.0f), glm::radians(89.0f))) * cosf(orbit.x),
            radius * sinf(glm::clamp(orbit.y, -glm::radians(89.0f), glm::radians(89.0f))),
            radius * cosf(glm::clamp(orbit.y, -glm::radians(89.0f), glm::radians(89.0f))) * sinf(orbit.x)
        );

        glm::vec3 forward = glm::normalize(target - cameraPos);
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
        glm::vec3 up = glm::cross(right, forward);

        target += -right * scrollDelta.x * panSpeed;
        target += up * scrollDelta.y * panSpeed;
    }

    scrollDelta *= 0.9f;
}

// ========================================================================

Scene::Scene() {}

Anchor Scene::createAnchor() {
    Anchor a = nextAnchorID++;
    anchors.push_back(a);
    registry.tags[a] = NameComponent{"New Anchor", AnchorType::None};
    return a;
}

void Scene::removeAnchor(Anchor a) {
    anchors.erase(std::remove(anchors.begin(), anchors.end(), a), anchors.end());
    registry.transforms.erase(a);
    registry.meshes.erase(a);
}

// ========================================================================

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!engine) return;

    engine->setSize(width, height);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!engine) return;

    engine->editorCamera.scrollDelta.x += (float)xoffset;
    engine->editorCamera.scrollDelta.y += (float)yoffset;
}

static Engine* g_boundEngine = nullptr;

Engine::Engine(const std::string& title, int width, int height, const std::source_location& loc) 
    : title(title.empty() ? "new engine" : title), width(width), height(height), running(true), window(nullptr) {
    
    if (!glfwInit()) {
        Message(3, "ENGINE", "Failed to initialize GLFW", loc.file_name(), loc.line());
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
        Message(3, "ENGINE", "Failed to create GLFW window", loc.file_name(), loc.line());
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Message(3, "ENGINE", "Failed to initialize GLAD", loc.file_name(), loc.line());
    }
    
    // initial scene / resources

    resources().loadDefaults();

    addScene("Scene1");

    viewport = std::make_unique<Viewport>(width, height);
    viewport->scene = activeScene;
    viewport->resize(720, 450);

    maxLights = 128;
    
    glGenBuffers(1, &lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Light) * maxLights, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);
    mappedPtr = (Light*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Light) * maxLights, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    enableBlend(true);
    
    glfwSwapInterval(0);

    glfwShowWindow(window);

}

Engine::~Engine() {
    if (window) glfwDestroyWindow(window);
    if (g_boundEngine == this) g_boundEngine = nullptr;
    glfwTerminate();
}

Scene* Engine::addScene(const std::string& name, const std::source_location& loc) {    
    auto it = scenes.find(name);
    if (it != scenes.end()) {
        Message(1, "SCENE", "Scene '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second.get();
    }
    
    auto scene = std::make_unique<Scene>();
    Scene* ptr = scene.get();
    scenes[name] = std::move(scene);
    activeScene = ptr;
    return ptr;
}

void Engine::removeScene(const std::string& name, const std::source_location& loc) {
    auto it = scenes.find(name);
    if (it != scenes.end()) {
        scenes.erase(it);
    } else {
        Message(2, "ENGINE", "Could not find scene '" + name + "'", loc.file_name(), loc.line());
    }   
}

void Engine::render() {

    if (!viewport->scene) return;

    viewport->bind();

    glViewport(0, 0, viewport->width, viewport->height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // CAMERA UPLOAD

    viewport->camera2 = Camera(editorCamera, 0.1f, 1000.0f, ((float)viewport->width / (float)viewport->height));

    for (Anchor a : viewport->scene->anchors) {
        
        if ( viewport->scene->registry.transforms.find(a) != viewport->scene->registry.transforms.end() && viewport->scene->registry.cameras.find(a) != viewport->scene->registry.cameras.end() ) {
            
            TransformComponent& t = viewport->scene->registry.transforms.find(a)->second;
            CameraComponent& c = viewport->scene->registry.cameras.find(a)->second;

            if (c.isMain) {
                viewport->camera2 = Camera(t, c, ((float)viewport->width / (float)viewport->height));
                break;
            }
        }

    }

    // LIGHT UPLOAD

    static std::vector<Light> lights;
    lights.clear();

    for (Anchor a : viewport->scene->anchors) {
        if ( viewport->scene->registry.transforms.find(a) != viewport->scene->registry.transforms.end() && viewport->scene->registry.lights.find(a) != viewport->scene->registry.lights.end() ) {
            
            TransformComponent& t = viewport->scene->registry.transforms.find(a)->second;
            LightComponent& l = viewport->scene->registry.lights.find(a)->second;

            Light gpu(t, l);

            lights.push_back(gpu);
        }
    }

    std::memcpy(mappedPtr, lights.data(), lights.size() * sizeof(Light));
    int numActiveLights = lights.size();

    // ANCHORS

    static std::vector<GLuint> updatedShaders;
    updatedShaders.clear();

    std::shared_ptr<Shader> shader = nullptr;

    for (Anchor a : viewport->scene->anchors) {
        if (viewport->scene->registry.transforms.find(a) == viewport->scene->registry.transforms.end()) continue;

        TransformComponent& t = viewport->scene->registry.transforms.find(a)->second;
        MeshComponent& m = viewport->scene->registry.meshes.find(a)->second;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), t.position) * glm::eulerAngleXYZ(t.rotationEuler.x, t.rotationEuler.y, t.rotationEuler.z) * glm::scale(glm::mat4(1.0f), t.scale);

        if (viewport->scene->registry.meshes.find(a) != viewport->scene->registry.meshes.end() && m.mesh != nullptr) {

            if (m.shader != nullptr) shader = m.shader;
            else shader = resources().shaders["default_basic"];

            if (std::find(updatedShaders.begin(), updatedShaders.end(), shader->ID) == updatedShaders.end()) {
                shader->activate();
                glUniform1i(glGetUniformLocation(shader->ID, "numLights"), numActiveLights);
                glUniform1f(glGetUniformLocation(shader->ID, "ambientLight"), 0.2f);
                glUniform1i(glGetUniformLocation(shader->ID, "enableLights"), true);
                viewport->camera2.uploadToShader(shader->ID);
                updatedShaders.push_back(shader->ID);
            }
            
            GLuint num = a;
            shader->activate();
            glUniform1ui(glGetUniformLocation(shader->ID, "objectID"), a);

            if (m.material != nullptr) m.material->uploadToShader(*shader); 
            else resources().materials["default_material"]->uploadToShader(*shader);

            m.mesh->draw(*shader, model);
        }
        
    }

    viewport->unbind();
}

void Engine::bind() { g_boundEngine = this; }

bool Engine::isRunning() const {
    return running && !glfwWindowShouldClose(window);
}

void Engine::closeWindow() {
    running = false;
}

void Engine::beginFrame() {

    g_renderStats.reset();

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    glfwPollEvents();
    editorCamera.ctrlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
    editorCamera.shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    ts.update();
    // update audio engine
    //set listener position to camera

    glViewport(0, 0, width, height);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // BE namespace
