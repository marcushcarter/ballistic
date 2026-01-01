#pragma once
#include "bepch.h"
#include "Renderer/DrawElementsIndirectCommand.h"

namespace ballistic
{
	class Window;
	class Scene;
    struct Vertex;

    struct RenderParameters {
        glm::mat4 camView;
        glm::mat4 camProj;
        alignas(16) glm::vec3 camPos;
    };

	class Renderer {
	public:
		Renderer() = default;
		~Renderer() { Shutdown(); }

		void ApplyWindowHints();

		bool Init(Window* window);
		void Shutdown();
		void Render(Scene* scene = nullptr);

		void SubmitCamera(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& pos);
		bool useMainCamera = false;

		void RequestResize(glm::vec2 dim);
		glm::vec2 GetSize() const { return m_currentSize; }

		GLuint GetNativeTextureHandle() { return m_outputTexture; }
		
        RenderParameters m_renderParams{};

	private:
		glm::vec2 m_currentSize{1, 1};
		glm::vec2 m_resizeSize{};
		bool m_pendingResize = true;

		// DEVICE
		
        GLuint m_mainFramebuffer = 0;
        GLuint m_outputTexture = 0;
        GLuint m_depthRbo = 0;
        GLuint m_blitShader = 0;

        GLuint m_vertexArray = 0;
        GLuint m_vertexBuffer = 0;
        GLuint m_indexBuffer = 0;
        size_t m_vertexCapacityBytes = 0;
        size_t m_indexCapacityBytes  = 0;

        GLuint m_instanceSSBO = 0; // SSBO binding 0
        size_t m_instanceCapacityBytes = 0;
        
        GLuint m_renderParamsUBO = 0; // UBO binding 0
        size_t m_renderParamsCapacityBytes = 0;

        void InitMeshBuffers();
        void InitShaderBuffers();
        void InitFramebuffer(uint32_t w, uint32_t h);
        void InitShaders();
        
        void EnsureVertexBuffer(const Vertex* data, size_t count);
        void EnsureIndexBuffer(const uint32_t* data, size_t count);
        void EnsureRenderParamsUBO(const RenderParameters* data);
        void EnsureInstanceSSBO(const glm::mat4* data, size_t count);

        GLuint CreateShader(const char* vertexSrc, const char* fragmentSrc);

        GLuint tempShader = 0;
	};

} // namespace ballistic
