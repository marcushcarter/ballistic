#pragma once

struct GLFWwindow;

namespace Ballistic
{
struct ImGuiLayer
{
    void Create(GLFWwindow* window);
    void Destroy();

    void NewFrame();
    void Render();
    void RecordDraw();
};
}