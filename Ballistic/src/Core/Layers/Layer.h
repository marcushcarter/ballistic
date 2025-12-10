#pragma once

#include "lrpch.h"

namespace Ballistic {

	class Layer {
	public:
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(void* e) {}
	};

}