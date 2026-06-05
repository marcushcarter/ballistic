#include <vk/core/surface.h>
#include <GLFW/glfw3native.h>

bool Surface::Create(VkInstance instance, GLFWwindow* window)
{
    Destroy();
    instanceHandle = instance;

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) { 
        // LOG_ERROR("glfwCreateWindowSurface failed");
        return false;
    }

    // LOG_DEBUG("Surface created");
    return true;
}

void Surface::Destroy()
{
    if (surface) {
        vkDestroySurfaceKHR(instanceHandle, surface, nullptr);
        surface = VK_NULL_HANDLE;
        // LOG_DEBUG("Surface destroyed");
    }
}
