#pragma once
#include "bepch.h"

namespace ballistic
{
    enum class EventType {
        None = 0,
        Test,
    };

	class IEvent
    {
	public:
		IEvent(EventType type) : m_type(type) {}
        virtual ~IEvent() = default;

		EventType getType() const { return m_type; }
        virtual std::string ToString() const { return "Event"; }

        bool handled = false;

	private:
		EventType m_type;
	};
    
} // namespace ballistic