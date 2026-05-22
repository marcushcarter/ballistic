#pragma once
#include "pch.h"
#include "Instance.h"

#include "Instance.h"
#include "DebugMessenger.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "Device.h"

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
