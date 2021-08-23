#pragma once

#include <Engine.Common/Wrapper.hpp>
#include "../Color/Color.hpp"
#include "../Texture/Texture.hpp"

namespace ember::engine::gfx {
    class Material {
    public:
        using value_type = Material;
        using reference_type = Material&;
        using const_reference_type = const Material&;

        /**
         * Destructor
         *
         * @author Julius
         * @date 17.11.2020
         */
        ~Material() = default;

    private:
        /**
         * Albedo
         */
        color _albedoColor = color { 1.F, 1.F, 1.F, 1.F };
        ptr<Texture> _albedoTexture = nullptr;

    private:
        /**
         * Alpha
         */
        ptr<Texture> _alphaTexture;

    private:
        /**
         * Ambient Occlusion
         */
        ptr<Texture> _aoTexture = nullptr;

    private:
        /**
         * Displacement
         */
        ptr<Texture> _displacementTexture;

    private:
        /**
         * Emission
         */
        color _emissionColor;
        ptr<Texture> _emissionTexture;

    private:
        /**
         * Metallic
         */
        ptr<Texture> _metallicTexture;

    private:
        /**
         * Normal
         */
        ptr<Texture> _normalTexture;

    private:
        /**
         * Roughness
         */
        ptr<Texture> _roughnessTexture;
    };

    /**
     * Equality operator
     *
     * @author Julius
     * @date 03.09.2020
     *
     * @param  left_ The first instance to compare.
     * @param  right_ The second instance to compare.
     *
     * @returns True if the parameters are considered equivalent.
     */
    [[nodiscard, maybe_unused]] inline bool operator ==(const Material& left_, const Material& right_) {
        if (&left_ == &right_)
            return true;
        return false;
    }
}
