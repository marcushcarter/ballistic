#pragma once
#include <glad/glad.h>
#include "State.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace gl {

    class Shader {
    public:
        Shader() = default;
        ~Shader() { destroy(); }

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        Shader(Shader&& other) noexcept { *this = std::move(other); }
        Shader& operator=(Shader&& other) noexcept {
            if (this != &other) {
                destroy();
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }

        void create() {
            if (m_id) return;
            m_id = glCreateProgram();
        }

        void destroy() {
            if (!m_id) return;
            glDeleteProgram(m_id);
            m_id = 0;
            m_attachedShaders.clear();
            m_uniformCache.clear();
        }

        bool valid() const { return m_id != 0 && glIsProgram(m_id); }
        GLuint get() const { return m_id; }

        void use() const { State::bindShader(m_id); }
        void unuse() const { State::bindShader(0); }

        GLuint attachShader(GLenum type, const char* source) {
            GLuint shader = glCreateShader(type);
            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);

            GLint success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                GLint length;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
                std::vector<char> infoLog(length);
                glGetShaderInfoLog(shader, length, nullptr, infoLog.data());
                std::cerr << "Shader compile error: " << infoLog.data() << std::endl;
            }

            glAttachShader(m_id, shader);
            m_attachedShaders.push_back(shader);
            return shader;
        }

        GLuint attachShader(GLenum type, const std::filesystem::path& filepath) {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                std::cerr << "Failed to open shader file: " << filepath << std::endl;
                return 0;
            }

            std::stringstream ss;
            ss << file.rdbuf();
            std::string sourceStr = ss.str();
            const char* source = sourceStr.c_str();

            return attachShader(type, source);
        }

        void detachShader(GLuint shader) {
            glDetachShader(m_id, shader);
            auto it = std::find(m_attachedShaders.begin(), m_attachedShaders.end(), shader);
            if (it != m_attachedShaders.end()) m_attachedShaders.erase(it);
        }

        void link() {
            glLinkProgram(m_id);

            GLint success;
            glGetProgramiv(m_id, GL_LINK_STATUS, &success);
            if (!success) {
                GLint length;
                glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);
                std::vector<char> infoLog(length);
                glGetProgramInfoLog(m_id, length, nullptr, infoLog.data());
                std::cerr << "Program link error: " << infoLog.data() << std::endl;
            }

            for (auto shader : m_attachedShaders)
                glDeleteShader(shader);

            m_attachedShaders.clear();
        }

        void validate() const {
            glValidateProgram(m_id);
            GLint status;
            glGetProgramiv(m_id, GL_VALIDATE_STATUS, &status);
            if (!status) {
                GLint length;
                glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);
                std::vector<char> infoLog(length);
                glGetProgramInfoLog(m_id, length, nullptr, infoLog.data());
                std::cerr << "Program validation failed: " << infoLog.data() << std::endl;
            }
        }

        template<typename T>
        void setUniform(const char* name, const T& value) const;

        template<> void setUniform<int>(const char* name, const int& value) const { glUniform1i(getUniformLocation(name), value); }
        template<> void setUniform<float>(const char* name, const float& value) const { glUniform1f(getUniformLocation(name), value); }
        template<> void setUniform<float[2]>(const char* name, const float (&value)[2]) const { glUniform2fv(getUniformLocation(name), 1, value); }
        template<> void setUniform<float[3]>(const char* name, const float (&value)[3]) const { glUniform3fv(getUniformLocation(name), 1, value); }
        template<> void setUniform<float[4]>(const char* name, const float (&value)[4]) const { glUniform4fv(getUniformLocation(name), 1, value); }
        template<> void setUniform<float[3][3]>(const char* name, const float (&value)[3][3]) const { glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]); }
        template<> void setUniform<float[4][4]>(const char* name, const float (&value)[4][4]) const { glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]); }

        void dispatchCompute(GLuint x, GLuint y, GLuint z, GLbitfield barriers = 0) const {
            use();
            glDispatchCompute(x, y, z);
            if (barriers) glMemoryBarrier(barriers);
        }

        GLuint getUniformLocation(const char* name) const {
            auto it = m_uniformCache.find(name);
            if (it != m_uniformCache.end()) return it->second;
            GLint loc = glGetUniformLocation(m_id, name);
            m_uniformCache[name] = loc;
            return loc;
        }

        void getActiveUniforms(std::vector<std::string>& outNames) const {
            GLint count;
            glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &count);
            GLint maxLen;
            glGetProgramiv(m_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
            std::vector<char> nameData(maxLen);

            for (GLint i = 0; i < count; i++) {
                GLsizei length;
                glGetActiveUniform(m_id, i, maxLen, &length, nullptr, nullptr, nameData.data());
                outNames.push_back(std::string(nameData.data(), length));
            }
        }

        GLint getAttribLocation(const char* name) const {
            return glGetAttribLocation(m_id, name);
        }

        void label(const char* name) {
            if (GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug)
                glObjectLabel(GL_PROGRAM, m_id, -1, name);
        }

    private:
        GLuint m_id{0};
        std::vector<GLuint> m_attachedShaders;
        mutable std::unordered_map<std::string, GLint> m_uniformCache;
    };

}