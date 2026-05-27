#pragma once
#include <cstdint>
#include <random>

constexpr uint64_t NULL_ID = 0;

inline uint64_t GenerateID()
{
    static std::mt19937_64 rng(std::random_device{}());
    static std::uniform_int_distribution<uint64_t> dist(1, INT64_MAX);
    return (uint64_t)dist(rng);
}