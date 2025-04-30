#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <cstring>

namespace PXTEngine {

    struct Buffer {
        uint8_t* bytes = nullptr;
        size_t size = 0;

        Buffer() = default;

        Buffer(size_t size) : size(size) {
            bytes = (uint8_t*) malloc(size);
        }

        // Constructor that copies data
        Buffer(const void* data, const size_t size) : size(size) {
            bytes = (uint8_t*) malloc(size);
            if (bytes) {
                memcpy(bytes, data, size);
            } else {
                this->size = 0; // Indicate allocation failure
            }
        }

        ~Buffer() {
            release();
        }

        // Copy Constructor (Deep Copy)
        Buffer(const Buffer& other) : size(other.size) {
            if (other.bytes) {
                bytes = (uint8_t*) malloc(size);
                if (bytes) {
                    memcpy(bytes, other.bytes, size);
                } else {
                    size = 0; 
                }
            } else {
                bytes = nullptr;
                size = 0;
            }
        }

        // Copy Assignment Operator (Rule of Three/Five)
        Buffer& operator=(const Buffer& other) {
            if (this != &other) { 
                release();

                size = other.size;
                if (other.bytes) {
                    bytes = (uint8_t*) malloc(size);
                    if (bytes) {
                        memcpy(bytes, other.bytes, size);
                    } else {
                        size = 0; 
                    }
                } else {
                    bytes = nullptr;
                    size = 0;
                }
            }
            return *this;
        }

        operator bool() const {
            return bytes != nullptr && size > 0;
        }

        void release() {
            if (bytes) {
                free(bytes);
                bytes = nullptr;
                size = 0;
            }
        }

        template<typename T>
        T* get() const {
            return (T*) bytes;
        }

        size_t getSize() const {
            return size;
        }
    };
    
}