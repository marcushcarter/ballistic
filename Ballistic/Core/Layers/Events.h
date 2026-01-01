#pragma once
#include "bepch.h"
#include "IEvent.h"

namespace ballistic
{
    struct TestEvent : public IEvent {
        int value;

        TestEvent(int value) : IEvent(EventType::Test), value(value) {}

        std::string ToString() const override {
            return "TestEvent: " + std::to_string(value);
        }
    };
    
    
} // namespace ballistic
