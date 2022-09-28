#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../vkinc.hpp"
#include "../RasterCullFace.hpp"
#include "../RasterPolyFront.hpp"
#include "../RasterPolyMode.hpp"
#include "../RasterSamples.hpp"

namespace ember::engine::gfx::pipeline {

    class RasterizationStage {
    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 01.12.2020
         */
        ~RasterizationStage() noexcept = default;

    private:
        /**
         * Polygon Mode
         */
        RasterPolyMode _polyMode = RasterPolyMode::eFace;

    public:
        /**
         * Polygon mode
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns A RasterPolyMode.
         */
        [[nodiscard]] RasterPolyMode polyMode() const noexcept;

        /**
         * Polygon mode
         *
         * @author Julius
         * @date 21.12.2020
         *
         * @returns A reference to a RasterPolyMode.
         */
        [[nodiscard]] RasterPolyMode& polyMode() noexcept;

    private:
        /**
         * Polygon Count -> Front Face
         */
        RasterPolyFront _polyFront = RasterPolyFront::eCounterClockwise;

    public:
        /**
         * Polygon front
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns A RasterPolyFront.
         */
        [[nodiscard]] RasterPolyFront polyFront() const noexcept;

        /**
         * Polygon front
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns A reference to a RasterPolyFront.
         */
        [[nodiscard]] ref<RasterPolyFront> polyFront() noexcept;

    private:
        /**
         * Face Culling
         */
        RasterCullFace _cullFace = RasterCullFace::eBack;

    public:
        /**
         * Cull face
         *
         * @author Julius
         * @date 23.12.2020
         *
         * @returns A RasterCullFace.
         */
        [[nodiscard]] RasterCullFace cullFace() const noexcept;

        /**
         * Cull face
         *
         * @author Julius
         * @date 23.12.2020
         *
         * @returns A reference to a RasterCullFace.
         */
        [[nodiscard]] ref<RasterCullFace> cullFace() noexcept;

    private:
        /**
         * Samples
         */
        RasterSamples _samples = RasterSamples::e1;

    public:
        /**
         * Gets the amount of requested Samples
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns The RasterSamples.
         */
        [[nodiscard]] RasterSamples samples() const noexcept;

    private:
        /**
         * Line Width
         */
        float _lineWidth = 1.F;

    public:
        /**
         * Line width
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns A float.
         */
        [[nodiscard]] float lineWidth() const noexcept;

    private:
        /**
         * Depth Check
         */
        bool _depthCheck = true;

    public:
        /**
         * Determines if we can depth check
         *
         * @author Julius
         * @date 23.12.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool depthCheck() const noexcept;

        /**
         * Depth check
         *
         * @author Julius
         * @date 23.12.2020
         *
         * @returns A reference to a bool.
         */
        [[nodiscard]] bool& depthCheck() noexcept;

    private:
        /**
         * Depth Write
         */
        bool _depthWrite = true;

    public:
        /**
         * Depth Write
         *
         * @author Julius
         * @date 19.03.2022
         *
         * @returns A reference to the write flag.
         */
        [[nodiscard]] bool& depthWrite() noexcept;

    private:
        /**
         * Depth Compare
         */
        vk::CompareOp _depthCompare = vk::CompareOp::eLessOrEqual;

    public:
        [[nodiscard]] vk::CompareOp& depthCompare() noexcept {
            return _depthCompare;
        }

    public:
        /**
         * Depth Bias
         */
        float _depthBiasConstant = 0.F;
        float _depthBiasClamp = 0.F;
        float _depthBiasSlope = 0.F;

    public:
        void setDepthBias(const float constant_, const float clamp_, const float slope_) {
            _depthBiasConstant = constant_;
            _depthBiasClamp = clamp_;
            _depthBiasSlope = slope_;
        }
    };
}
