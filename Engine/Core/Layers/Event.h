#pragma once
#include "bepch.h"

namespace Ballistic {

	enum class EventType {};

	class Event {
	public:
		Event(EventType type, void* data = nullptr)
			: m_type(type), m_data(data) {}

		EventType getType() const { return m_type; }
		void* getData() const { return m_data; }

	private:
		EventType m_type;
		void* m_data;
	};
}