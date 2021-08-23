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

// Descriptor Defines
#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1

#define DESC_BIND_OBJ_UNIFORM_BUFFER 0
#define DESC_BIND_OBJ_SAMPLER 1
#define DESC_BIND_OBJ_SHADOW 2
#define DESC_BIND_OBJ_FRAG_UNIFORM_BUFFER 3
#define DESC_BIND_OBJ_FRAG_STORAGE_BUFFER 4

#define DESC_BIND_SKY_UNIFORM_BUFFER 0
#define DESC_BIND_SKY_SAMPLER 1

#define DESC_BIND_TERRA_UNIFORM_BUFFER 0
#define DESC_BIND_TERRA_BLEND_SAMPLER 1
#define DESC_BIND_TERRA_SAMPLER 2
#define DESC_BIND_TERRA_SHADOW 3
#define DESC_BIND_TERRA_FRAG_UNIFORM_BUFFER 4
#define DESC_BIND_FRAG_TERRA_STORAGE_BUFFER 5

// Temporary forward function declaration
namespace clockwork::__internal::gfx::utils {
    inline void _updateDescriptorBuffer(const vk::DescriptorSet& set_, vk::WriteDescriptorSet& write_,
        const uint32_t bind_, const vk::DescriptorBufferInfo* desc_, const uint32_t count_ = 1) {
        write_.setPNext(nullptr);
        write_.setDstSet(set_);
        write_.setDstBinding(bind_);
        write_.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        write_.setDescriptorCount(count_);
        write_.setPBufferInfo(desc_);
    }

    inline void _updateDescriptorSampler(const vk::DescriptorSet& set_, vk::WriteDescriptorSet& write_,
        const uint32_t bind_, const vk::DescriptorImageInfo& desc_) {
        write_.setPNext(nullptr);
        write_.setDstSet(set_);
        write_.setDstBinding(bind_);
        write_.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        write_.setDescriptorCount(1);
        write_.setPImageInfo(&desc_);
    }

    inline void _updateDescriptorStorage(const vk::DescriptorSet& set_, vk::WriteDescriptorSet& write_,
        const uint32_t bind_, const vk::DescriptorBufferInfo* desc_, const uint32_t count_ = 1) {
        write_.setPNext(nullptr);
        write_.setDstSet(set_);
        write_.setDstBinding(bind_);
        write_.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        write_.setDescriptorCount(count_);
        write_.setPBufferInfo(desc_);
    }
}

/*

### Pool 0 - General
1. [Buffer] ubo { mat4 mvp; }

### Pool 0 - Terrain
2. [Sampler] skybox
3. [Sampler] heightmap
4. [Sampler] blendmap

### Pool 0 - Materials
6. [Buffer] meta
7. [Sampler] albedo
8. [Sampler] normal
9. [Sampler] metalness
10. [Sampler] roughness
11. [Sampler] ao
12. [Sampler] alpha
13. [Sampler] specular

### Pool 1 - General
1. [Buffer] ubo { mat4 mvp; }

### Pool 1 - Mrt
2. [Sampler] position
3. [Sampler] normal
4. [Sampler] albedo
5. [Sampler] metalness
6. [Sampler] roughness
7. [Sampler] ao
8. [Sampler] specular
9. [Sampler] depth

 */
