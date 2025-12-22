#pragma once
#include "bepch.h"

namespace Ballistic {

    struct GUID {
        uint64_t value;
        static constexpr uint64_t Invalid = UINT64_MAX;

        constexpr GUID() noexcept
            : value(Invalid) {}

        explicit GUID(uint64_t id) noexcept
            : value(id == Invalid ? Generate() : id) {}

        constexpr bool operator==(const GUID& other) const noexcept { return value == other.value; }
        constexpr bool operator!=(const GUID& other) const noexcept { return value != other.value; }

        constexpr bool valid() const noexcept { return value != Invalid; }

    private:
        static uint64_t Generate() noexcept {
            static std::atomic<uint64_t> counter { 1 };
            static thread_local std::mt19937_64 rng { std::random_device{}() };
            uint64_t c = counter.fetch_add(1, std::memory_order_relaxed);
            uint64_t r = rng();
            return (r ^ (c + 0x9E3779B97F4A7C15ULL)) | 1ULL;
        }
    };
}

namespace std {
    template<>
    struct hash<Ballistic::GUID> {
        std::size_t operator()(const Ballistic::GUID& guid) const noexcept {
            return std::hash<uint64_t>()(guid.value);
        }
    };
}