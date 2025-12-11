#pragma once

#include "bepch.h"

namespace Ballistic {

	enum class EventType {
		TestEvent
	};

	class Event {
	public:
		Event(EventType type, void* data = nullptr) : m_Type(type), m_Data(data) {}

		EventType getType() const { return m_Type; }
		void* getData() const { return m_Data; }

	private:
		EventType m_Type;
		void* m_Data;
	};
}