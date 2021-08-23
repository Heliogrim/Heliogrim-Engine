#pragma once
#include "../vkinc.hpp"
#include "../RasterPolyMode.hpp"
#include "../InputRate.hpp"
#include "../RasterPolyFront.hpp"
#include "../RasterSamples.hpp"
#include "../RasterCullFace.hpp"
#include "../PrimitiveTopology.hpp"
#include "../TextureFormat.hpp"

namespace ember::engine::gfx::api {

    /**
     * Translate internal to vulkan
     *
     * @author Julius
     * @date 25.01.2021
     *
     * @param  mode_ The mode.
     *
     * @returns A vk::PolygonMode.
     */
    [[nodiscard]] vk::PolygonMode vkTranslateMode(RasterPolyMode mode_);

    /**
     * Translate internal to vulkan
     *
     * @author Julius
     * @date 25.01.2021
     *
     * @param  front_ The front.
     *
     * @returns A vk::FrontFace.
     */
    [[nodiscard]] vk::FrontFace vkTranslateFace(RasterPolyFront front_);

    /**
     * Translate internal to vulkan
     *
     * @author Julius
     * @date 25.01.2021
     *
     * @param  samples_ The samples.
     *
     * @returns The vk::SampleCountFlagBits.
     */
    [[nodiscard]] vk::SampleCountFlagBits vkTranslateSamples(RasterSamples samples_);

    /**
     * Translate internal to vulkan
     *
     * @author Julius
     * @date 25.01.2021
     *
     * @param  rate_ The rate.
     *
     * @returns A vk::VertexInputRate.
     */
    [[nodiscard]] vk::VertexInputRate vkTranslateInputRate(InputRate rate_);

    /**
     * Translate internal to vulkan
     *
     * @author Julius
     * @date 25.01.2021
     *
     * @param  face_ The face.
     *
     * @returns The vk::CullModeFlagBits.
     */
    [[nodiscard]] vk::CullModeFlagBits vkTranslateCull(RasterCullFace face_);

    /**
     * Translate internal to vulkan
     *
     * @author Julius
     * @date 25.01.2021
     *
     * @param  topology_ The topology.
     *
     * @returns A vk::PrimitiveTopology.
     */
    [[nodiscard]] vk::PrimitiveTopology vkTranslateTopology(PrimitiveTopology topology_);

    /**
     * Translate internal to vulkan
     *
     * @author Julius
     * @date 30.01.2021
     *
     * @param  format_ Describes the format to use.
     *
     * @returns A vk::Format.
     */
    [[nodiscard]] vk::Format vkTranslateFormat(TextureFormat format_);

    /**
     * Translate vulkan to internal
     *
     * @author Julius
     * @date 01.02.2021
     *
     * @param  format_ Describes the format to use.
     *
     * @returns A TextureFormat.
     */
    [[nodiscard]] TextureFormat vkTranslateFormat(vk::Format format_);

}
