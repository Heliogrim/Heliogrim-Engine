#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::render {
    class macro_novtable LightCaptureInterface {
    public:
        using this_type = LightCaptureInterface;

    public:
        constexpr LightCaptureInterface() noexcept = default;

        constexpr virtual ~LightCaptureInterface() noexcept = default;

    public:
        virtual void store(mref<struct SpotLightCaptureBatch>) noexcept = 0;

        virtual void store(mref<struct DirectionalLightCaptureBatch>) noexcept = 0;

        virtual void store(mref<struct PointLightCaptureBatch>) noexcept = 0;
    };
}
