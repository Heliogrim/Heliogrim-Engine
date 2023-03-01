#pragma once

#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx::shader {
    enum class ShaderType: u8 {
        eNone = 0b0,
        //
        eVertex = 0b1 << 0,
        eGeometry = 0b1 << 1,
        eTessellation = 0b1 << 2,
        eFragment = 0b1 << 3,
        //
        eCompute = 0b1 << 5,
        //
        eAll = 0b11111111
    };

    typedef Flag<ShaderType> ShaderTypes;

    enum class BindingType: u8 {
        eUndefined = 0x0,
        //
        eImageSampler = 0x1,
        //
        eUniformBuffer = 0x4,
        //
        eStorageBuffer = 0x7
    };

    enum class BindingUpdateInterval : u8 {
        /**
         * Will be concatenated with descriptors, where update interval is undefined
         */
        eUndefined = 0x0,

        /**
         * Will be concatenated with descriptors, which are update on [Event]
         */
        eVertexUpdate = 0x2,
        eIndexUpdate = 0x3,
        eMaterialUpdate = 0x4,

        /**
         * Will generate one descriptor set per binding
         */
        eDedicated = 0x6,

        /**
         * Will be concatenated with descriptors, which are updated per [Frame, Instance]
         */
        ePerFrame = 0x7,
        ePerInstance = 0x8
    };
}
