#pragma once
#include "pch.h"
#include "instance.h"
#include "debug_messenger.h"
#include "surface.h"
#include "physical_device.h"
#include "queue.h"
#include "device.h"

struct Context
{
    Instance instance;
    DebugMessenger debugMessenger;
    Surface surface;
    PhysicalDevice physicalDevice;
    Device device;
    Queue graphicsQueue, presentQueue, transferQueue, computeQueue;
    bool hasAsyncCompute = false;
    uint32_t frameCount = 0;

    bool Create(GLFWwindow* window);
    void Destroy();
};
