#pragma once
#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.GFX/Buffer/BufferLikeObject.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace hg::engine::render {
    namespace {
        struct alignas(16) LightBufferDataLayout140 {
            alignas(16) math::fvec3 color;
            alignas(16) math::fvec3 position;
            alignas(16) math::fvec3 direction;
            alignas(16) float limit;
        };

        struct alignas(16) LightBufferDataLayout430 {
            alignas(16) math::fvec3 color;
            alignas(16) math::fvec3 position;
            alignas(16) math::fvec3 direction;
            alignas(16) float limit;
        };

        // TODO: Check whether we want to decompose the view matrix into location and rotation (sizeof(float) * 16 -> sizeof(float) * 2 * 3)
        struct alignas(16) ViewBufferDataLayout140 {
            /* View-Projection-Matrix */
            alignas(16) math::fmat4 vpm;
            /* View-Matrix */
            alignas(16) math::fmat4 vm;
            /* Projection-Matrix */
            alignas(16) math::fmat4 pm;
        };

        struct alignas(16) ViewBufferDataLayout430 {
            /* View-Projection-Matrix */
            alignas(16) math::fmat4 vpm;
            /* View-Matrix */
            alignas(16) math::fmat4 vm;
            /* Projection-Matrix */
            alignas(16) math::fmat4 pm;
        };
    }

    struct ViewBuffer {
        using layout_std140_type = ViewBufferDataLayout140;
        using layout_std430_type = ViewBufferDataLayout430;

        gfx::BufferLikeObject blo;
    };
}

namespace hg::engine::serialization {
    template <>
    inline void DataLayout<render::ViewBufferDataLayout140>::describe() {

        using namespace ::hg::engine::serialization::layout;

        const auto f32Layout = make_sptr<DataLayout<float>>();
        defineSpan(offsetof(render::ViewBufferDataLayout140, vpm), f32Layout, sizeof(math::fmat4) / sizeof(float));
        defineSpan(offsetof(render::ViewBufferDataLayout140, vm), f32Layout, sizeof(math::fmat4) / sizeof(float));
        defineSpan(offsetof(render::ViewBufferDataLayout140, pm), f32Layout, sizeof(math::fmat4) / sizeof(float));
    }

    template <>
    inline void DataLayout<render::ViewBufferDataLayout430>::describe() {

        using namespace ::hg::engine::serialization::layout;

        const auto f32Layout = make_sptr<DataLayout<float>>();
        defineSpan(offsetof(render::ViewBufferDataLayout430, vpm), f32Layout, sizeof(math::fmat4) / sizeof(float));
        defineSpan(offsetof(render::ViewBufferDataLayout430, vm), f32Layout, sizeof(math::fmat4) / sizeof(float));
        defineSpan(offsetof(render::ViewBufferDataLayout430, pm), f32Layout, sizeof(math::fmat4) / sizeof(float));
    }
}
