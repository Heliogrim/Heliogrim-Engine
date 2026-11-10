#pragma once

#include <Engine.Common/stdafx.h>

// Vulkan Definition for Graphics
#include "vkinc.hpp"

#define DEFAULT_FENCE_TIMEOUT UINT64_MAX

// Texture Formats
#define COLOR_FORMAT vk::Format::eR8G8B8A8Unorm
#define SHADOW_DEPTH_FORMAT vk::Format::eD32Sfloat

// Cascaded Shadow
#define DEFAULT_SHADOW_CASCADE_COUNT 4
#define DEFAULT_SHADOW_CASCADE_LAMBDA .97F

#define TEX_SHADOW_DIM 2048

// Bias
#define DEPTH_BIAS_CONST 1.25F
#define DEPTH_BIAS_SLOPE 1.75F

// Camera
const float FOV = 70.0F;
const float FAR_PLANE = 1000.0F;
const float NEAR_PLANE = 0.01F;

#define FIRST_PERSON

// Defined algorithm
#define LOD_ALGO_FIRST_LAST (std::sqrt(2))
#define LOD_ALGO(var_, last_) (std::ceil(std::exp(var_ + 1) / last_))
