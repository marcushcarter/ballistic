#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <functional>
#include <compare>

namespace ballistic {

struct Guid
{
    static constexpr uint64_t INVALID = 0;

    static constexpr size_t CHARS = 16;
    static constexpr size_t BUFFER = CHARS + 1;
    static constexpr size_t PATH_CHARS = 2 + 1 + CHARS;
    static constexpr size_t PATH_BUFFER = PATH_CHARS + 1;

    uint64_t value = INVALID;

    static Guid generate();
    static Guid from_string(std::string_view p_text);

    uint8_t shard() const { return (uint8_t)(value >> 56); }

    void to_chars(char* r_out) const;
    void to_path_chars(char* r_out) const;
    std::string to_string() const;

    friend bool operator==(Guid, Guid) = default;
    friend std::strong_ordering operator<=>(Guid, Guid) = default;
};

static_assert(sizeof(Guid) == 8);
static_assert(std::is_trivially_copyable_v<Guid>);
static_assert(std::is_standard_layout_v<Guid>);

}

template<>
struct std::hash<ballistic::Guid> {
    size_t operator()(ballistic::Guid p_guid) const noexcept {
        return (size_t)p_guid.value;
    }
};