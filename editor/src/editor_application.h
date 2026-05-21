#pragma once
#include "pch.h"
#include <core/application.h>

struct EditorApplication : Application
{
    void OnInit() override;
    void OnUpdate() override;
    void OnShutdown() override;
};