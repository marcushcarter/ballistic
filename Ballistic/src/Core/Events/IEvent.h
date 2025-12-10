#pragma once

#include "lrpch.h"

namespace Ballistic {

	class IEvent {
	public:
		virtual ~IEvent() = default;

    	virtual const char* GetName() const = 0;
	};
}