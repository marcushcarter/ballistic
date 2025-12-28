#pragma once
#include "Platform/API/OpenGL/Shader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gl {

inline void setUniform(Shader& shader, const std::string& name, const glm::mat2& mat) {
    glUniformMatrix2fv(shader.getUniformLocation(name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::mat3& mat) {
    glUniformMatrix3fv(shader.getUniformLocation(name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::mat4& mat) {
    glUniformMatrix4fv(shader.getUniformLocation(name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::vec2& vec) {
    glUniform2fv(shader.getUniformLocation(name.c_str()), 1, glm::value_ptr(vec));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::vec3& vec) {
    glUniform3fv(shader.getUniformLocation(name.c_str()), 1, glm::value_ptr(vec));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::vec4& vec) {
    glUniform4fv(shader.getUniformLocation(name.c_str()), 1, glm::value_ptr(vec));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::ivec2& vec) {
    glUniform2iv(shader.getUniformLocation(name.c_str()), 1, glm::value_ptr(vec));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::ivec3& vec) {
    glUniform3iv(shader.getUniformLocation(name.c_str()), 1, glm::value_ptr(vec));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::ivec4& vec) {
    glUniform4iv(shader.getUniformLocation(name.c_str()), 1, glm::value_ptr(vec));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::uvec2& vec) {
    glUniform2uiv(shader.getUniformLocation(name.c_str()), 1, glm::value_ptr(vec));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::uvec3& vec) {
    glUniform3uiv(shader.getUniformLocation(name.c_str()), 1, glm::value_ptr(vec));
}

inline void setUniform(Shader& shader, const std::string& name, const glm::uvec4& vec) {
    glUniform4uiv(shader.getUniformLocation(name.c_str()), 1, glm::value_ptr(vec));
}

}