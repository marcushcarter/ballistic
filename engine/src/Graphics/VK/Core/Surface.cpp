#include "Surface.h"

bool Surface::Create(VkInstance instance, GLFWwindow* window)
{
    // VK_CHECK_HANDLE(instance, VkInstance, false);
    // CHECK_PTR(window, "Invlaid GLFWwindow", false);

    Destroy();
    instanceHandle = instance;

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) { 
        LOG_ERROR("glfwCreateWindowSurface failed", false);
        return false;
    }

    LOG_DEBUG("Surface created");
    return true;
}

void Surface::Destroy()
{
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instanceHandle, surface, nullptr);
        surface = VK_NULL_HANDLE;
        instanceHandle = VK_NULL_HANDLE;
        LOG_DEBUG("Surface destroyed");
    }
}
