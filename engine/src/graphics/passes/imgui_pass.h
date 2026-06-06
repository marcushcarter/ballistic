#pragma once
struct RenderGraph;
struct Renderer;
struct ImGuiLayer;
struct FrameGraph;
void AddImGuiPass(RenderGraph& g, FrameGraph& fg, Renderer* renderer, ImGuiLayer* imguiLayer, bool hasProject);
