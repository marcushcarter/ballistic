#pragma once
#include <glad/glad.h>
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

        void create();
        void destroy();

        bool valid() const { return m_id != 0 && glIsProgram(m_id); }
        void validate() const;
        GLuint get() const { return m_id; }

        void use() const;
        void unuse() const;

        GLuint attachShader(GLenum type, const char* source);
        GLuint attachShader(GLenum type, const std::filesystem::path& filepath);
        void detachShader(GLuint shader);
        void link();

        template<typename T>
        void setUniform(const char* name, const T& value) const;

        template<> void setUniform<int>(const char* name, const int& value) const { glUniform1i(getUniformLocation(name), value); }
        template<> void setUniform<float>(const char* name, const float& value) const { glUniform1f(getUniformLocation(name), value); }
        template<> void setUniform<float[2]>(const char* name, const float (&value)[2]) const { glUniform2fv(getUniformLocation(name), 1, value); }
        template<> void setUniform<float[3]>(const char* name, const float (&value)[3]) const { glUniform3fv(getUniformLocation(name), 1, value); }
        template<> void setUniform<float[4]>(const char* name, const float (&value)[4]) const { glUniform4fv(getUniformLocation(name), 1, value); }
        template<> void setUniform<float[3][3]>(const char* name, const float (&value)[3][3]) const { glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]); }
        template<> void setUniform<float[4][4]>(const char* name, const float (&value)[4][4]) const { glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]); }

        void dispatchCompute(GLuint x, GLuint y, GLuint z, GLbitfield barriers = 0) const;

        GLuint getUniformLocation(const char* name) const;
        void getActiveUniforms(std::vector<std::string>& outNames) const;
        GLint getAttribLocation(const char* name) const;

        void label(const char* name);

    private:
        GLuint m_id{0};
        std::vector<GLuint> m_attachedShaders;
        mutable std::unordered_map<std::string, GLint> m_uniformCache;
    };

}