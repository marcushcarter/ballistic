#pragma once
#include "bepch.h"
#include "Root/LogManager/Log.h"

namespace ballistic
{
    struct LayerContext;

	class ILayer
    {
	public:
		ILayer(LayerContext& context, const std::string name = "Layer")
            : m_context(context), m_name(name) {}

		virtual ~ILayer() = default;

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnEvent(IEvent& e) = 0;

		const std::string& getName() const { return m_name; }

    protected:
        LayerContext& m_context;

	private:
		std::string m_name;
	};
    
} // namespace ballistic