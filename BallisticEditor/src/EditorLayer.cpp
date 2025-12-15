#include "EditorLayer.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

#include "Panels/DemoPanel/DemoPanel.h"

namespace Ballistic {

	void EditorLayer::createImGuiDescriptorPool() {
		std::vector<vk::DescriptorPoolSize> poolSizes = {
			{vk::DescriptorType::eSampler, 1000},
			{vk::DescriptorType::eCombinedImageSampler, 1000},
			{vk::DescriptorType::eSampledImage, 1000},
			{vk::DescriptorType::eStorageImage, 1000},
			{vk::DescriptorType::eUniformTexelBuffer, 1000},
			{vk::DescriptorType::eStorageTexelBuffer, 1000},
			{vk::DescriptorType::eUniformBuffer, 1000},
			{vk::DescriptorType::eStorageBuffer, 1000},
			{vk::DescriptorType::eUniformBufferDynamic, 1000},
			{vk::DescriptorType::eStorageBufferDynamic, 1000},
			{vk::DescriptorType::eInputAttachment, 1000}
		};

		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		poolInfo.maxSets = 1000 * static_cast<uint32_t>(poolSizes.size());
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		m_ImGuiDescriptorPool = m_VkRenderer->m_Device->createDescriptorPoolUnique(poolInfo);
	}

	void EditorLayer::uploadFonts() {
		// auto cmdBuf = m_CommandBuffer.get();
		// ImGui_ImplVulkan_CreateFontsTexture(cmdBuf);
		// m_VkRenderer->flushCommandBuffer(cmdBuf);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void EditorLayer::onAttach() {
		IMGUI_CHECKVERSION();
	    ImGui::CreateContext();
	    ImGuiIO& io = ImGui::GetIO();
	    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
	    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	    ImGui::StyleColorsDark();

		switch (WindowAPI::GetAPI()) {
			case WindowAPI::API::GLFW:
				ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(m_Window->get()), true);
				break;
		}
		
    	createImGuiDescriptorPool();

	    ImGui_ImplVulkan_InitInfo init_info{};
	    init_info.Instance = m_VkRenderer->m_Instance.get();
	    init_info.PhysicalDevice = m_VkRenderer->m_PhysicalDevice;
	    init_info.Device = m_VkRenderer->m_Device.get();
	    // init_info.Queue = m_VkRenderer->m_GraphicsQueue;
	    init_info.DescriptorPool = *m_ImGuiDescriptorPool;
	    // init_info.MinImageCount = m_VkRenderer->m_Swapchain->GetMinImageCount();
	    // init_info.ImageCount = m_VkRenderer->m_Swapchain->GetImageCount();
		init_info.PipelineInfoMain.Subpass = 0;
	    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		// initInfo.Allocator = nullptr;
		// initInfo.CheckVkResultFn = [](VkResult err) { if (err != VK_SUCCESS) abort(); };

		init_info.PipelineInfoMain.RenderPass = m_VkRenderer->m_RenderPass.get();

	    ImGui_ImplVulkan_Init(&init_info);

		uploadFonts();

	    ImGui::CreateContext();

		m_Panels.push_back(std::make_unique<DemoPanel>());
		m_Panels.push_back(std::make_unique<DemoPanel>());

	    for (auto& panel : m_Panels)
        	panel->init();
	}

	void EditorLayer::onDetach() {
		vkDeviceWaitIdle(m_VkRenderer->m_Device.get());
		ImGui_ImplVulkan_Shutdown();
		
		switch (WindowAPI::GetAPI()) {
			case WindowAPI::API::GLFW:
				ImGui_ImplGlfw_Shutdown();
				break;
		}
	}

	void EditorLayer::GenDockspace() {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
	    ImGui::SetNextWindowPos(viewport->Pos);
	    ImGui::SetNextWindowSize(viewport->Size);
	    ImGui::SetNextWindowViewport(viewport->ID);

	    ImGuiWindowFlags window_flags =
	        ImGuiWindowFlags_NoDocking |
	        ImGuiWindowFlags_NoTitleBar |
	        ImGuiWindowFlags_NoCollapse |
	        ImGuiWindowFlags_NoResize |
	        ImGuiWindowFlags_NoMove |
	        ImGuiWindowFlags_NoBringToFrontOnFocus |
	        ImGuiWindowFlags_NoNavFocus |
	        ImGuiWindowFlags_MenuBar;

	    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	    ImGui::Begin("dockspace", nullptr, window_flags);
	    
	    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
	    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
	    
	    ImGui::PopStyleVar(3);
	    ImGui::End();
	}

	void EditorLayer::onUpdate() {
	}

	void EditorLayer::onEvent(void* ePtr) {
		Event* e = static_cast<Event*>(ePtr);
	}
}