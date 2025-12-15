#pragma once

#include "bepch.h"
#include "Core/IWindow.h"

namespace Ballistic {

	class VkRenderer {
	public:
		VkRenderer() = default;
		explicit VkRenderer(IWindow* window) : m_Window(window) {}

		void Init();
		void Shutdown();
		void Render();
		
		vk::UniqueInstance m_Instance;
	    vk::PhysicalDevice m_PhysicalDevice;
	    vk::UniqueDevice m_Device;
	    vk::Queue m_GraphicsQueue;
	    uint32_t m_GraphicsQueueFamilyIndex;
	    vk::UniqueSurfaceKHR m_Surface;
	    vk::UniqueSwapchainKHR m_Swapchain;
	    std::vector<vk::Image> m_SwapchainImages;
	    vk::Format m_SwapchainImageFormat;
	    vk::Extent2D m_SwapchainExtent;
	    std::vector<vk::UniqueImageView> m_SwapchainImageViews;

	    vk::UniqueRenderPass m_RenderPass;
		std::vector<vk::UniqueFramebuffer> m_Framebuffers;
		vk::UniqueCommandPool m_CommandPool;
		std::vector<vk::UniqueCommandBuffer> m_CommandBuffers;

	private:

	    IWindow* m_Window = nullptr;

	};
}