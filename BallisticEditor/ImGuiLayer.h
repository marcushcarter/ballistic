#pragma once
#include <Ballistic.h>

struct ImGuiContext;

namespace ballistic
{
    class ImGuiLayer : public ILayer
    {
    public:
        ImGuiLayer(const LayerContext& context = {}, const std::string& name = "ImGuiLayer");
        ~ImGuiLayer() override { OnDetach(); }

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;
        void OnEvent(IEvent& e) override;

    private:
        
        // PANEL STACK
    };
    
} // namespace ballistic
