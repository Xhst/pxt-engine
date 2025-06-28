#pragma once

#include <string>

const std::string SPV_SHADERS_PATH = "../out/shaders/";
const std::string SHADERS_PATH = "../assets/shaders/";
const std::string MODELS_PATH = "../assets/models/";
const std::string TEXTURES_PATH = "../assets/textures/";

const std::string IMGUI_INI_FILEPATH = "../assets/imgui_config/imgui.ini";

const std::string WHITE_PIXEL = "pixel_0xFFFFFFFF_RGBA8_SRGB";
const std::string WHITE_PIXEL_LINEAR = "pixel_0xFFFFFFFF_RGBA8_LINEAR";
const std::string GRAY_PIXEL_LINEAR = "pixel_0xFF808080_RGBA8_LINEAR";
const std::string BLACK_PIXEL_LINEAR = "pixel_0xFF000000_RGBA8_LINEAR";
const std::string NORMAL_PIXEL_LINEAR = "pixel_0xFFFF8080_RGBA8_LINEAR";

const std::string DEFAULT_MATERIAL = "default_material";

namespace CubeFace {
	constexpr uint32_t RIGHT = 0;
	constexpr uint32_t LEFT = 1;
	constexpr uint32_t TOP = 2;
	constexpr uint32_t BOTTOM = 3;
	constexpr uint32_t BACK = 4;
	constexpr uint32_t FRONT = 5;
}
