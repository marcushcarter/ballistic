#pragma once

#include "bepch.h"
#include "Platform/Vulkan/VulkanInstance.h"

namespace Ballistic {

	class VulkanRenderer {
	public:
		VulkanRenderer() {}
		~VulkanRenderer() {}

		void Init() {}
		void Shutdown() {}

		VulkanInstance& getInstance() { return *vulkanInstance; }

	private:
		std::unique_ptr<VulkanInstance> vulkanInstance;
	};
}