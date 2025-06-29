#pragma once
 
#include "core/pch.hpp"
 
namespace PXTEngine {
 
    /**
     * @brief Combine multiple hash values into a single hash value
     * 
     * @tparam T Type of the first value
     * @tparam Rest Types of the other values
     * @param seed Hash value where the final hash will be stored
     * @param v First value to hash
     * @param rest Other values that will be hashed
     */
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {

        // Combine the hash of the current value with the seed
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        // Recursively combine the hash of the other values
        (hashCombine(seed, rest), ...);
    };
 
}