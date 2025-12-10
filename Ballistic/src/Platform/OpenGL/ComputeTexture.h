#pragma once

#include "lrpch.h"

class ComputeTexture {
public:
	ComputeTexture(int width, int height, GLenum internalFormat = GL_RGBA32F);
	~ComputeTexture();

	void bind(GLuint unit) const;
	void unbind(GLuint unit) const;

	GLuint m_RendererID;
	int m_Width, m_Height;
	GLenum m_InternalFormat;

private:
	static inline std::unordered_map<GLuint, GLuint> s_Bindings;
};
