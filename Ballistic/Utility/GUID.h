#pragma once
#include "bepch.h"

namespace ballistic
{
    struct GUID 
    {
        uint64_t value;

        static constexpr uint64_t InvalidValue = UINT64_MAX;
        static const GUID Invalid;

        constexpr GUID() noexcept
            : value(InvalidValue) {}

        explicit constexpr GUID(uint64_t id) noexcept
            : value(id) {}

        static GUID Create() noexcept {
            return GUID(Generate());
        }

        constexpr bool valid() const noexcept { return value != InvalidValue; }
        constexpr bool operator==(const GUID& other) const noexcept { return value == other.value; }
        constexpr bool operator!=(const GUID& other) const noexcept { return value != other.value; }

    private:
        static uint64_t Generate() noexcept {
            static std::atomic<uint64_t> counter{ 1 };
            static thread_local std::mt19937_64 rng{ std::random_device{}() };

            uint64_t c = counter.fetch_add(1, std::memory_order_relaxed);
            uint64_t r = rng();

            uint64_t id = (r ^ (c * 0x9E3779B97F4A7C15ULL)) | 1ULL;
            return id == InvalidValue ? id - 1 : id;
        }
    };
}

namespace std
{
    template<>
    struct hash<ballistic::GUID> {
        std::size_t operator()(const ballistic::GUID& guid) const noexcept {
            return std::hash<uint64_t>()(guid.value);
        }
    };

} // namespace ballistic
