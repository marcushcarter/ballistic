#pragma once

#include "lrpch.h"
#include "Core/Events/IEvent.h"
#include "../../Platform/OpenGL/Image2D.h"

namespace Ballistic {

	class FrameRenderedEvent : public IEvent {
	public:
		FrameRenderedEvent(std::shared_ptr<Image2D> img) : image(img) {}

		const char* GetName() const override { return "FrameRenderedEvent"; }

		std::shared_ptr<Image2D> image;
	};
}