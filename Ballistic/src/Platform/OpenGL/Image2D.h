#pragma once

#include "lrpch.h"

class Image2D {
public:
	Image2D(int width, int height, GLenum internalFormat = GL_RGBA32F);
	~Image2D();
	
	void bind(GLuint unit) const;
	void unbind(GLuint unit) const;

	GLuint m_RendererID;
	int m_Width, m_Height;
	GLenum m_InternalFormat;

private:
	static inline std::unordered_map<GLuint, GLuint> s_Bindings;
};
