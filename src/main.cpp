#include "engine.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    
    CGEngine::Engine engine;
    
    try {
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}