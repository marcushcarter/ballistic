#pragma once
struct RenderGraph;
struct Renderer;
struct ImGuiLayer;
void AddImGuiPass(RenderGraph& g, Renderer* renderer, ImGuiLayer* imguiLayer);
