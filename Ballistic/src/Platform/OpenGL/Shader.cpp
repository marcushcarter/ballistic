#include "Platform/OpenGL/Shader.h"

Shader::Shader() {
    m_RendererID = glCreateProgram();
}

Shader::~Shader() {
    if (m_RendererID) {
        glDeleteProgram(m_RendererID);
        m_RendererID = 0;
    }
}

void Shader::attachSource(GLenum stage, const char* source) {
    m_ShaderSources[stage] = std::string(source);
}

void Shader::attachFile(GLenum stage, const char* path) {
    m_ShaderSources[stage] = loadFile(path);
}

GLuint Shader::compile(GLenum stage, const char* source) {
    GLuint shader = glCreateShader(stage);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        throw std::runtime_error(std::string("Shader compilation failed:\n") + log);
    }

    return shader;
}

bool Shader::link() {

    m_CompiledShaders.clear();

    for (auto& [stage, source] : m_ShaderSources) {
        GLuint shader = compile(stage, source.c_str());
        m_CompiledShaders.push_back(shader);
        glAttachShader(m_RendererID, shader);
    }

    glLinkProgram(m_RendererID);

    GLint success;
    glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(m_RendererID, 1024, nullptr, log);
        std::cout << "Shader link failed:\n" << log << std::endl;

        for (GLuint s : m_CompiledShaders) glDeleteShader(s);
        m_CompiledShaders.clear();
        return false;
    }

    for (GLuint s : m_CompiledShaders) glDeleteShader(s);
    m_CompiledShaders.clear();
    m_ShaderSources.clear();

    return true;
}

void Shader::dispatchCompute(int x, int y, int z) {
    bind();
    m_WorkGroupsX = x;
    m_WorkGroupsY = y;
    m_WorkGroupsZ = z;
    glDispatchCompute(x, y, z);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Shader::bind() const {
    if (s_Current != m_RendererID) {
        glUseProgram(m_RendererID);
        s_Current = m_RendererID;
    }
}

void Shader::unbind() const {
    if (s_Current != 0) {
        glUseProgram(0);
        s_Current = 0;
    }
}

GLint Shader::getUniformLocation(const char* name) {
    std::string n(name);

    auto it = m_UniformCache.find(n);
    if (it != m_UniformCache.end()) return it->second;

    GLint location = glGetUniformLocation(m_RendererID, n.c_str());
    m_UniformCache[n] = location;
    return location;
}

void Shader::setUniform(const char* name, bool v) {
    bind();
	GLint loc = getUniformLocation(name);
	if (loc != -1) glUniform1i(loc, v);
}

void Shader::setUniform(const char* name, int v) {
    bind();
	GLint loc = getUniformLocation(name);
	if (loc != -1) glUniform1i(loc, v);
}

void Shader::setUniform(const char* name, float v) {
    bind();
	GLint loc = getUniformLocation(name);
	if (loc != -1) glUniform1f(loc, v);
}

void Shader::setUniform(const char* name, const glm::vec2& v) {
    bind();
    GLint loc = getUniformLocation(name);
    if (loc != -1) glUniform2fv(loc, 1, &v.x);
}

void Shader::setUniform(const char* name, const glm::vec3& v) {
    bind();
    GLint loc = getUniformLocation(name);
    if (loc != -1) glUniform3fv(loc, 1, &v.x);
}

void Shader::setUniform(const char* name, const glm::vec4& v) {
    bind();
    GLint loc = getUniformLocation(name);
    if (loc != -1) glUniform4fv(loc, 1, &v.x);
}

void Shader::setUniform(const char* name, const glm::mat4& m) {
    bind();
    GLint loc = getUniformLocation(name);
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::setLabel(const char* name) const {
	glObjectLabel(GL_PROGRAM, m_RendererID, -1, name);
}

std::string Shader::loadFile(const char* f) {
    std::string filepath(f);
    std::ifstream stream(filepath);

    std::stringstream ss;
    std::string line;
    while (getline(stream, line)) {
        ss << line << '\n';
    }

    std::string source = ss.str();
    return source;
}