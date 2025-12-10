#pragma once

#include "lrpch.h"

class Shader {
public:
	Shader();
	~Shader();

	void attachSource(GLenum stage, const char* source);
	void attachFile(GLenum stage, const char* path);
	bool link();

	void dispatchCompute(int x, int y = 1, int z = 1);

	void bind() const;
	void unbind() const;

	GLint getUniformLocation(const char* name);

	void setUniform(const char* name, bool v);
    void setUniform(const char* name, int v);
    void setUniform(const char* name, float v);
    void setUniform(const char* name, const glm::vec2& v);
    void setUniform(const char* name, const glm::vec3& v);
    void setUniform(const char* name, const glm::vec4& v);
    void setUniform(const char* name, const glm::mat4& m);

    void setLabel(const char* name) const;

	GLuint m_RendererID;
	int m_WorkGroupsX, m_WorkGroupsY, m_WorkGroupsZ;

private:
	std::unordered_map<std::string, GLint> m_UniformCache;
    std::unordered_map<GLenum, std::string> m_ShaderSources;
    std::vector<GLuint> m_CompiledShaders;
    
    static inline GLuint s_Current = 0;

    GLuint compile(GLenum stage, const char* source);
    std::string loadFile(const char* filepath);
};