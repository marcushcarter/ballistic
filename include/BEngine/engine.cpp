#include "BEngine/engine.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <source_location>
#include <chrono>
#include <cstring>

namespace BE {

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

float FrameTime::update() {
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

void VAO::bind() { glBindVertexArray(ID); }

void VAO::unbind() { glBindVertexArray(0); }

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

void VBO::bind() { glBindBuffer(GL_ARRAY_BUFFER, ID); }

void VBO::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

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

void EBO::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); }

void EBO::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

// ========================================================================

Framebuffer::Framebuffer(int w, int h) : width(w), height(h) { filter = GL_NEAREST; createFramebuffer(); }

Framebuffer::~Framebuffer() { destroyFramebuffer(); }

void Framebuffer::createFramebuffer() {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("FRamebuffer not complete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroyFramebuffer() {
    if (rbo) glDeleteRenderbuffers(1, &rbo);
    if (texture) glDeleteTextures(1, &texture);
    if (fbo) glDeleteFramebuffers(1, &fbo);
}

void Framebuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, fbo); glViewport(0, 0, width, height); }

void Framebuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); /* reset veiwport to window size */ }

void Framebuffer::bindTexture(GLuint shaderID, const char* uniform, int unit) {
    glUseProgram(shaderID);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shaderID, uniform), unit);
}

void Framebuffer::resize(int newWidth, int newHeight, bool linearFilter) {
    width = newWidth;
    height = newHeight;
    filter = linearFilter ? GL_LINEAR : GL_NEAREST;
    destroyFramebuffer();
    createFramebuffer();
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

void Shader::activate() { glUseProgram(ID); }

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
    // if (loc == -1) std::cout << "Failed to find 'Uniform Sampler2D " << uniform << "'" << std::endl;
    glUniform1i(loc, slot);
}

void Texture::bind(GLuint slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

// ========================================================================

bool g_cullEnabled = false;

void setCullFace(bool enable) {
    if (enable && !g_cullEnabled) {
        glEnable(GL_CULL_FACE);
        g_cullEnabled = true;
    } else if (!enable && g_cullEnabled) {
        glDisable(GL_CULL_FACE);
        g_cullEnabled = false;
    }
}

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

    setCullFace(cull);

    if (diffuseMap) {
        diffuseMap->setUniformUnit(shader.ID, "diffuseMap", 0);
        diffuseMap->bind(0);
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

Mesh::Mesh(const std::vector<Vertex>& verts, const std::vector<GLuint>& inds, const std::vector<Texture>& texs)
    : vertices(verts), indices(inds), textures(texs) {

    vao.bind();

    vbo = new VBO(vertices);
    ebo = new EBO(indices);

    vbo->linkVertexAttrib(0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    vbo->linkVertexAttrib(1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    vbo->linkVertexAttrib(2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    vbo->linkVertexAttrib(3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
    
    vao.unbind();
}

Mesh::Mesh(const std::string& objPath)
    : vertices(), indices(), textures() {
    loadOBJ(objPath.c_str());
}

Mesh::Mesh(const std::string* objSource)
    : vertices(), indices(), textures() {
    loadOBJSource(objSource);
}

Mesh::~Mesh() {
    delete vbo;
    delete ebo;
    
    for (auto& tex : textures) {
        tex.~Texture();
    }
}

void Mesh::draw(Shader& shader, const glm::mat4& modelMatrix) {
    shader.activate();
    vao.bind();

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "uModel"), 1, GL_FALSE, &modelMatrix[0][0]);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    vao.unbind();
}

void Mesh::makePreview(Framebuffer& fb, Shader& shader, glm::vec2 rotation) {

    setCullFace(false);

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

    // glm::mat4 model = glm::translate(glm::mat4(1.0f), -center) * glm::scale(glm::mat4(1), glm::vec3(1.0f)) * glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0,1,0)) * glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1,0,0));
    glm::mat4 view = glm::lookAt(cameraPos * (maxExtent * 2.0f), glm::vec3(0), glm::vec3(0,1,0));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, maxExtent * 10.0f);

    shader.activate();

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "uView"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "uProjection"), 1, GL_FALSE, &proj[0][0]);

    draw(shader, model);

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
    this->textures = { Texture("diffuse", 4, 4, BE::Default::FallbackTexture) };

    vao.bind();
    delete vbo;
    delete ebo;
    vbo = new VBO(this->vertices);
    ebo = new EBO(this->indices);

    vbo->linkVertexAttrib(0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    vbo->linkVertexAttrib(1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    vbo->linkVertexAttrib(2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    vbo->linkVertexAttrib(3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));

    vao.unbind();

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
    this->textures = { Texture("diffuse", 4, 4, BE::Default::FallbackTexture) };

    vao.bind();
    delete vbo;
    delete ebo;
    vbo = new VBO(this->vertices);
    ebo = new EBO(this->indices);

    vbo->linkVertexAttrib(0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    vbo->linkVertexAttrib(1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    vbo->linkVertexAttrib(2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    vbo->linkVertexAttrib(3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));

    vao.unbind();

    Message(0, "MESH", "OBJ loaded successfully", "SOURCE", lineNum);
}

// ========================================================================

std::shared_ptr<Mesh> ResourceManager::loadMesh(const std::string& name, const std::vector<Vertex>& verts, const std::vector<GLuint>& inds, const std::vector<Texture>& texs, const std::source_location& loc) {
    auto it = meshes.find(name);
    if (it != meshes.end()) {
        Message(1, "RESOURCE", "Mesh '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second;
    }
    
    auto mesh = std::make_shared<Mesh>(verts, inds, texs);
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
    loadMesh("__cube", "include/BEngine/meshes/cube.obj");
    loadMesh("__quad", "include/BEngine/meshes/quad.obj");

    loadShaderDSL("include/BEngine/shaders/core/mesh_preview.dsl");
    loadShaderDSL("include/BEngine/shaders/core/default_scene.dsl");
    loadShaderDSL("include/BEngine/shaders/core/flat_color.dsl");
    loadShaderDSL("include/BEngine/shaders/post/blit.dsl");

    auto texture = std::make_shared<Texture>("diffuse", 4, 4, BE::Default::FallbackTexture);
    textures["Fallback"] = texture;

    loadTexture("Box", "res/textures/box.png", "diffuse");
    loadTexture("Box_specular", "res/textures/box_specular.png", "diffuse");

    materials["__default_material"] = std::make_shared<Material>();
    materials["__default_material"]->diffuseMap = textures["Fallback"].get();
    
    materials["material bond"] = std::make_shared<Material>();
    materials["material bond"]->diffuseMap = textures["Box"].get();
    materials["material bond"]->normalMap = textures["Box_specular"].get();
    materials["material bond"]->cull = true;
}

// ========================================================================

Camera::Camera(const std::string& cameraName, int width, int height, float fov, float nearPlane, float farPlane, const glm::vec3& pos, const glm::vec3& dir) 
    : name(cameraName.empty() ? "new camera" : cameraName), width(width), height(height), fov(fov), nearPlane(nearPlane), farPlane(farPlane), position(pos), zoom(1.0f) {

    glm::vec3 forward = glm::normalize(dir);
    glm::vec3 defaultForward = glm::vec3(0.0f, 0.0f, -1.0f);
    orientation = glm::rotation(defaultForward, forward);

    projectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), nearPlane, farPlane);
    orthoMatrix = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);

    updateViewMatrix();
}

void Camera::rotate(const glm::vec3& axis, float angle) {
    glm::quat qrot = glm::angleAxis(angle, axis);
    orientation = glm::normalize(qrot * orientation);
}

void Camera::handleInputs(GLFWwindow* window, float dt, bool focusing) {
    static bool mouseLookActive = false;
    static double lastX = 0, lastY = 0;

    float speed = 2.5f;
    float sensitivity = 1.5f;

    glm::vec3 move(0.0f);
    glm::vec3 tmp(0.0f);
    glm::vec3 forward, right, up;

    forward = orientation * glm::vec3(0, 0, -1);
    right   = orientation * glm::vec3(1, 0, 0);
    up      = orientation * glm::vec3(0, 1, 0);

    glm::vec3 flatForward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
    glm::vec3 flatRight   = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
    glm::vec3 flatUp      = glm::vec3(0.0f, 1.0f, 0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += flatForward * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= flatForward * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= flatRight * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += flatRight * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) move += flatUp * speed * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) move -= flatUp * speed * dt;

    position += move;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mouseLookActive && focusing) {
        mouseLookActive = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &lastX, &lastY);
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && mouseLookActive && focusing) {
        mouseLookActive = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (mouseLookActive && focusing) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float offsetX = static_cast<float>(mouseX - lastX);
        float offsetY = static_cast<float>(lastY - mouseY);

        lastX = mouseX;
        lastY = mouseY;

        yaw   -= offsetX * 0.002f * sensitivity;
        pitch += offsetY * 0.002f * sensitivity;

    } else {
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  yaw   += dt * sensitivity;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yaw   -= dt * sensitivity;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    pitch += dt * sensitivity;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  pitch -= dt * sensitivity;
    }

    if (pitch > glm::radians(89.9f)) pitch = glm::radians(89.9f);
    if (pitch < glm::radians(-89.9f)) pitch = glm::radians(-89.9f);

    glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1,0,0));
    glm::quat qYaw   = glm::angleAxis(yaw,   glm::vec3(0,1,0));

    orientation = glm::normalize(qYaw * qPitch);
}

void Camera::updateViewMatrix() {
    glm::vec3 forward = orientation * glm::vec3(0, 0, -1);
    glm::vec3 up      = orientation * glm::vec3(0, 1,  0);
    glm::vec3 target  = position + forward;

    viewMatrix = glm::lookAt(position, target, up);

    projectionMatrix = glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), nearPlane, farPlane);

    projViewMatrix = projectionMatrix * viewMatrix;
    
    orthoMatrix = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);

}

void Camera::uploadToShader(GLuint shaderID) {
    // glm::mat4 mvp = projViewMatrix * modelMatrix;

    // glUniformMatrix4fv(glGetUniformLocation(shaderID, "uMVP"), 1, GL_FALSE, &mvp[0][0]);
    // glUniformMatrix4fv(glGetUniformLocation(shaderID, "uModel"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uView"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "uProjection"), 1, GL_FALSE, &projectionMatrix[0][0]);

    glUniform3fv(glGetUniformLocation(shaderID, "camPos"), 1, &position[0]);
}

// ========================================================================

Light::Light(float type, const glm::vec3 pos, const glm::vec3 dir, const glm::vec3 col, float inten, float pad1_ ) {
    position = glm::vec4(pos, (float)type);
    color = glm::vec4(col, inten);
    direction = glm::vec4(dir, pad1_);
    shadowMatrices[0] = glm::mat4(1.0f);
    shadowMatrices[1] = glm::mat4(1.0f);
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
        float fov    = glm::radians(45.0f); // adjust if you want wider cone
        float aspect = 1.0f;
        float range  = direction.w;
        glm::mat4 proj = glm::perspective(fov, aspect, 0.1f, range);
        glm::mat4 view = glm::lookAt(pos, pos + dir, glm::vec3(0, 1, 0));
        shadowMatrices[0] = proj * view;
    }


}

void Light::setPosition(const glm::vec3& pos) { position = glm::vec4(pos, position.w); }

void Light::setColor(const glm::vec3& col) { color = glm::vec4(col, color.w); }

void Light::setIntensity(float intensity) { color.w = intensity; }

void Light::setDirection(const glm::vec3& dir) { direction = glm::vec4(dir, direction.w); }

// ========================================================================

LightManager::LightManager(size_t maxLights) 
    : maxLights(maxLights) {

    // lightMesh = new Mesh("Light Mesh", {}, {}, {});
    // lightMesh.loadOBJ("res/models/cube.obj");
    
    glGenBuffers(1, &lightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Light) * maxLights, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);
    mappedPtr = (Light*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Light) * maxLights, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

LightManager::~LightManager() {
    if (lightSSBO) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glDeleteBuffers(1, &lightSSBO);
    }
}

void LightManager::bind() { glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO); }

void LightManager::updateGPU() { std::memcpy(mappedPtr, lights.data(), lights.size() * sizeof(Light)); }

void LightManager::uploadToShader(GLuint shaderID) { glUniform1i(glGetUniformLocation(shaderID, "numLights"), (int)lights.size()); }

// void LightManager::updateActiveLightsForObject(const glm::vec3& objPos, float objRadius) {
//     activeLights.clear();
//
//     for (auto& light : lights) {
//         if (light.position.w == 0.0f) {
//             activeLights.push_back(light);
//         } 
//         else {
//             float distance = glm::length(glm::vec3(light.position) - objPos);
//             float range = light.direction.w;
//             if (distance <= objRadius + range)
//                 activeLights.push_back(light);
//         }
//     }
//
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
//     std::memcpy(mappedPtr, activeLights.data(), activeLights.size() * sizeof(Light));
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
// }

void LightManager::generateAllMatrices() {
    for (auto& light : lights) {
        light.generateMatrices();
    }
    updateGPU();
}

size_t LightManager::addLight(const std::string& name, int type, const std::source_location& loc) {
    Light light(type);
    if (lights.size() < maxLights) {
        size_t index = lights.size();
        lights.push_back(light);
        lights.back().generateMatrices();
        lightLookup[name] = index;
        updateGPU();
        return index;
    }
    Message(1, "LIGHT", "No space left for light '" + name + "' in light manager", loc.file_name(), loc.line());\
    return SIZE_MAX;
}

void LightManager::removeLight(const std::string& name, int type, const std::source_location& loc) {
    auto it = lightLookup.find(name);
    if (it != lightLookup.end()) {
        size_t index = it->second;
        lights.erase(lights.begin() + index);
        lightLookup.erase(it);

        for (auto& [key, value] : lightLookup) {
            if (value > index) value--;
        }

        updateGPU();
    } else {
        Message(2, "LIGHT", "Could not find light '" + name + "'", loc.file_name(), loc.line());
    }   
}

Light* LightManager::getLight(const std::string& name, const std::source_location& loc) {
    auto it = lightLookup.find(name);
    if (it != lightLookup.end()) {
        return &lights[it->second];
    } else {
        Message(2, "LIGHT", "Could not find light '" + name + "'", loc.file_name(), loc.line());
        return nullptr;
    }
}

// ========================================================================

// ========================================================================

// ========================================================================

// ========================================================================

// ========================================================================

// ========================================================================

// ========================================================================

// ========================================================================

Scene::Scene() : lightManager(128) { addCamera("Camera1"); }

Camera* Scene::addCamera(const std::string& name, const std::source_location& loc) {
    auto it = cameras.find(name);
    if (it != cameras.end()) {
        Message(1, "SCENE", "Camera '" + name + "' already exists", loc.file_name(), loc.line());
        return it->second.get();
    }
    
    auto camera = std::make_unique<Camera>(name);
    Camera* ptr = camera.get();
    cameras[name] = std::move(camera);
    activeCamera = ptr;
    return ptr;
}

void Scene::removeCamera(const std::string& name, const std::source_location& loc) {
    auto it = cameras.find(name);
    if (it != cameras.end()) {
        // if (activeCamera == cameras[index]) activeCamera = nullptr;
        cameras.erase(it);
        // activeCamera = cameras[0];
    } else {
        Message(2, "SCENE", "Could not find camera '" + name + "'", loc.file_name(), loc.line());
    }   
}

// ========================================================================

// ========================================================================

// ========================================================================

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!engine) return;

    engine->setSize(width, height);

    // Message(0, "ENGINE", "Framebuffer resized", __FILE__, __LINE__);
}

void drop_callback(GLFWwindow* window, int count, const char** paths) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!engine) return;

    for (int i = 0; i < count; i++) {
        std::string path = paths[i];
        std::string filename = std::filesystem::path(path).stem().string();
        
        if (path.ends_with(".obj")) { engine->resources().loadMesh(filename, path); }
        
        if (path.ends_with(".dsl") || path.ends_with(".besl")) { engine->resources().loadShaderDSL(path); }
        
        if (path.ends_with(".png") || path.ends_with(".jpg")) { engine->resources().loadTexture(filename, path, "diffuse"); }
        
        if (path.ends_with(".mtl")) { engine->resources().loadMaterial(filename, path); }
        
    }
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

    viewport = std::make_unique<Viewport>();
    viewport.get()->scene = activeScene;
    viewport.get()->camera = activeScene->activeCamera;
    viewport.get()->resize(720, 450);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    g_cullEnabled = false;
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
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

// std::unique_ptr<Scene> Engine::addScene(const std::string& name, const std::source_location& loc) {
//     auto it = scenes.find(name);
//     if (it != scenes.end()) {
//         Message(1, "ENGINE", "Scene '" + name + "' already exists", loc.file_name(), loc.line());
//         return it->second;
//     }
//
//     auto scene = std::make_shared<Scene>(name);
//     scenes[name] = scene;
//     return scene;
// }

// void Engine::removeScene(const std::string& name, const std::source_location& loc) {
//     auto it = scenes.find(name);
//     if (it != scenes.end()) {
//         scenes.erase(it);
//     } else {
//         Message(2, "ENGINE", "Could not find scene '" + name + "'", loc.file_name(), loc.line());
//     }   
// }

// std::shared_ptr<Scene> Engine::getScene(const std::string& name, const std::source_location& loc) {
//     auto it = scenes.find(name);
//     if (it != scenes.end()) {
//         return it->second;
//     } else {
//         Message(2, "ENGINE", "Could not find scene '" + name + "'", loc.file_name(), loc.line());
//         return nullptr;
//     }
// }

void Engine::renderViewportTexture(Viewport& vp) {

    if (!vp.camera || !vp.scene) return;

    vp.framebuffer.bind();
    
    if ((vp.framebuffer.width != vp.width) || (vp.framebuffer.height != vp.height) && vp.width > 0 && vp.height > 0) 
        vp.framebuffer.resize(vp.width, vp.height);

    glViewport(0, 0, vp.framebuffer.width, vp.framebuffer.height);
    // glClearColor(0.1,0.1,0.1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vp.camera->width = vp.framebuffer.width;
    vp.camera->height = vp.framebuffer.height;

    std::shared_ptr<Shader> shader = nullptr;
    std::shared_ptr<Mesh> mesh = nullptr;

    // GAME OBJECTS

    if (vp.scene->customShader) shader = vp.scene->customShader; 
    else shader = resources().shaders["__scene"];
    mesh = resources().meshes["Test Scene"];

    shader->activate();
    vp.scene->lights().uploadToShader(shader->ID);
    vp.camera->uploadToShader(shader->ID);
    glm::mat4 model = glm::mat4(1.0f);
    // mesh->draw(*shader, model, false);

    for (Anchor a : vp.scene->anchors) {
        if (vp.scene->registry.transforms.find(a) == vp.scene->registry.transforms.end()) continue;

        TransformComponent& t = vp.scene->registry.transforms.find(a)->second;
        MeshComponent& m = vp.scene->registry.meshes.find(a)->second;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), t.position) * glm::eulerAngleXYZ(t.rotation.x, t.rotation.y, t.rotation.z) * glm::scale(glm::mat4(1.0f), t.scale);

        if (vp.scene->registry.meshes.find(a) != vp.scene->registry.meshes.end() && m.mesh != nullptr && m.shader != nullptr) {
            
            if (m.material != nullptr) m.material->uploadToShader(*m.shader); 
            else resources().materials["__default_material"]->uploadToShader(*m.shader);

            m.mesh->draw(*m.shader, model);
            continue;
        }
        
    }

    // CAMERAS

    // shader = resources().shaders["__flat_color"];
    // mesh = resources().meshes["__cube"];

    // shader->activate();
    // vp.camera->uploadToShader(shader->ID);
    // glUniform4fv(glGetUniformLocation(shader->ID, "uColor"), 1, glm::value_ptr(glm::vec4(1)));
    
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // for (auto& [key, camera] : vp.scene->cameras) {
    //     glm::mat4 model = glm::translate(glm::mat4(1.0f), camera->position) * glm::mat4_cast(camera->orientation) * glm::scale(glm::mat4(1.0f), glm::vec3(0.25f));
    //     mesh->draw(*shader, model, false);
    // }

    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // LIGHTS

    // shader = resources().shaders["__flat_color"];
    // mesh = resources().meshes["__cube"];

    // shader->activate();
    // vp.camera->uploadToShader(shader->ID);
    // GLuint colorLoc = glGetUniformLocation(shader->ID, "uColor");

    // for (int i = 0; i < vp.scene->lights().lights.size(); i++) {
    //     const auto& light = vp.scene->lights().lights[i];
    //     glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(light.position)) * glm::eulerAngleXYZ(light.direction.x, light.direction.y, light.direction.z) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));   
    //     glUniform4fv(colorLoc, 1, glm::value_ptr(light.color));
    //     mesh->draw(*shader, model);
    // }

    vp.framebuffer.unbind();
}

// void Engine::drawTexture(Texture& texture, int x, int y, int width, int height) {}

void Engine::bind() { g_boundEngine = this; }

bool Engine::isRunning() const {
    return running && !glfwWindowShouldClose(window);
}

void Engine::closeWindow() {
    running = false;
}

void Engine::beginFrame() {

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetDropCallback(window, drop_callback);
    
    glfwPollEvents();
    // inputs
    frameTime.update();
    // update audio engine
    //set listener position to camera

    glViewport(0, 0, width, height);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // BE namespace