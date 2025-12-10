#pragma once

#include "lrpch.h"
#include "Core/Events/IEvent.h"

namespace Ballistic {

	class Layer {
	public:
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(std::shared_ptr<IEvent> event) {}
	};

}