#pragma once

#include <memory>

namespace CGEngine {

    /**
     * @brief Alias for std::unique_ptr to simplify usage.
     * 
     * A smart pointer that exclusively owns a dynamically allocated object.
     * The object is destroyed when the unique_ptr goes out of scope or is reset.
     * Ownership cannot be shared, but it can be transferred.
     * 
     * @tparam T Type of the object to be managed by the unique pointer.
     */
    template<typename T>
    using Unique = std::unique_ptr<T>;

    /**
     * @brief Creates a std::unique_ptr with the provided arguments.
     * 
     * This function is a wrapper around std::make_unique, which constructs 
     * an object of type T and wraps it in a std::unique_ptr. It forwards 
     * all provided arguments to the constructor of T.
     * 
     * @tparam T Type of the object to be created.
     * @tparam Args Parameter pack for the constructor arguments of T.
     * @param args Arguments to be forwarded to the constructor of T.
     * @return A std::unique_ptr managing the newly created object.
     */
    template<typename T, typename ... Args>
    constexpr Unique<T> createUnique(Args&& ... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    /**
     * @brief Alias for std::shared_ptr to simplify usage.
     * 
     * A smart pointer that manages a dynamically allocated object 
     * with shared ownership. The object is destroyed when the last 
     * shared_ptr managing it is destroyed or reset.
     * 
     * @tparam T Type of the object to be managed by the shared pointer.
     */
    template<typename T>
    using Shared = std::shared_ptr<T>;

    /**
     * @brief Creates a std::shared_ptr with the provided arguments.
     * 
     * This function is a wrapper around std::make_shared, which constructs 
     * an object of type T and wraps it in a std::shared_ptr. It forwards 
     * all provided arguments to the constructor of T. The object is reference 
     * counted, and shared ownership is allowed.
     * 
     * @tparam T Type of the object to be created.
     * @tparam Args Parameter pack for the constructor arguments of T.
     * @param args Arguments to be forwarded to the constructor of T.
     * @return A std::shared_ptr managing the newly created object.
     */
    template<typename T, typename ... Args>
    constexpr Shared<T> createShared(Args&& ... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

}
