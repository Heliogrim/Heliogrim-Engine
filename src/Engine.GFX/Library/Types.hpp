#pragma once

#include <Engine.Common/stdafx.h>
#include <Engine.Common/Math/Vector.hpp>

namespace ember::engine::gfx::types {
    struct BoundImage {
        BoundImage(const vk::Image& image_, const vk::DeviceMemory& memory_) {
            this->image = image_;
            this->memory = memory_;
        }

        vk::Image image;
        vk::DeviceMemory memory;
    };

    // Shader Equal
    #define LIGHT_COUNT 1
    #define SHADOW_MAP_CASCADE_COUNT 4

    // Scene
    struct UniformBuffer {
        ember::math::mat4 proj;
        ember::math::mat4 view;
        ember::math::mat4 model;
        ember::math::mat4 mvp;
    };

    struct UniformBufferObjFrag {
        ember::math::vec4 camPos;
        ember::math::vec4 skyColor;
        std::array<float, SHADOW_MAP_CASCADE_COUNT> splits = std::array<float, SHADOW_MAP_CASCADE_COUNT>();
        std::array<ember::math::mat4, SHADOW_MAP_CASCADE_COUNT> casMvp = std::array<ember::math::mat4,
            SHADOW_MAP_CASCADE_COUNT>();
    };

    // Compute
    struct UniformBufferObjComp {
        ember::math::mat4 proj;
        ember::math::mat4 view;
        ember::math::vec4 camPos;
        std::array<ember::math::vec4, 6> planes;
    };

    struct UniformIndirectStat {
        uint8_t count;
    };

    // Skybox
    ALIGNED(
        struct UniformBufferObjSky {
        ember::math::mat4 mvp;
        },
        16
    );

    // Offscreen
    struct UniformBufferObjOff {
        std::array<ember::math::mat4, SHADOW_MAP_CASCADE_COUNT> mvp;
    };

    struct Cascade {
        vk::Framebuffer framebuffer;
        vk::DescriptorSet descSet;
        vk::ImageView view;

        float depth = 0.F;
        ember::math::mat4 mvp;
    };

    struct ImageBuffer {
        vk::Image image;
        vk::ImageView view;
    };

    struct SwapBuffer : ImageBuffer {
        vk::Framebuffer frame;
    };

    struct DepthBuffer : ImageBuffer {
        vk::Sampler sampler;
        vk::Format format = vk::Format::eUndefined;
        vk::DeviceMemory memory;
    };

    struct VertexBuffer {
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        vk::Semaphore sema;
    };

    struct QueueFamilyIndices {
        int32_t computeFamily = -1;
        int32_t transferFamily = -1;
        int32_t graphicsFamily = -1;

        bool isComplete() {
            return graphicsFamily >= 0 && computeFamily >= 0 && transferFamily >= 0;
        }
    };

    struct ThreadConstData {
        ember::math::mat4 transformMatrix;
    };
}
