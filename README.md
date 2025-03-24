# PXT Engine

PXT Engine is a custom game engine built with C++, utilizing Vulkan for high-performance rendering, GLFW for window and input handling, and GLM for mathematics. The engine also integrates various third-party libraries like ImGui, tinyobjloader, and EnTT for ECS-based game architecture.

## Prerequisites
Before building, ensure you have the following installed:
- **CMake** (version 3.11.0 or higher)
- **Vulkan SDK** (Ensure `VULKAN_SDK_PATH` is set)
- **MinGW (if using MinGW Makefiles on Windows)**
- **C++17 Compiler**

## Setup & Build Instructions

### Windows (MinGW)
1. Clone the repository:
   ```sh
   git clone --recursive https://github.com/Prox747/pxt-engine
   cd pxt_engine
   ```
2. Set up environment variables in `.env` (optional but recommended):
   ```sh
   MINGW_PATH=C:/mingw64
   VULKAN_SDK_PATH=C:/VulkanSDK/x.x.x.x
   ```
3. Run the `start.bat` script to build and run the project.

### Linux/Unix
1. Clone the repository:
   ```sh
   git clone --recursive https://github.com/Prox747/pxt-engine
   cd pxt_engine
   ```
2. Install dependencies:
   ```sh
   sudo apt install build-essential cmake vulkan-sdk
   ```
3. Run the `start.bat` script to build and run the project.

## Shader Compilation
The engine automatically compiles shaders using `glslangValidator`. Ensure the Vulkan SDK is properly installed and accessible. All `.frag` and `.vert` shaders in `assets/shaders/` are compiled into SPIR-V and stored in `build/shaders/`.
When the project is built with the start script it will automatically compile the shaders.