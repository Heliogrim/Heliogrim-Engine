#pragma once
#include "../Device/Device.hpp"
#include "../Texture/Texture.hpp"
#include "Engine.GFX/Geometry/Mesh.hpp"

namespace ember::engine::gfx {
    class Skybox {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 23.12.2020
         *
         * @param  device_ The device.
         */
        Skybox(cref<sptr<Device>> device_);

        /**
         * Gets the geometry
         *
         * @author Julius
         * @date 23.12.2020
         *
         * @returns A ptr&lt;const Mesh&gt;
         */
        ptr<const Mesh> geometry() const;

        /**
         * Gets the geometry
         *
         * @author Julius
         * @date 23.12.2020
         *
         * @returns A ptr&lt;const Mesh&gt;
         */
        ptr<const Mesh> geometry();

        /**
         * Gets the cubemap
         *
         * @author Julius
         * @date 23.12.2020
         *
         * @returns A cref&lt;Texture&gt;
         */
        cref<Texture> cubemap() const;

        /**
         * Gets the cubemap
         *
         * @author Julius
         * @date 23.12.2020
         *
         * @returns A ref&lt;Texture&gt;
         */
        ref<Texture> cubemap();

    private:
        /**
         * Device
         */
        sptr<Device> _device;

    private:
        /**
         * Skybox Mesh
         */
        ptr<Mesh> _geometry;

    private:
        /**
         * Cubemap
         */
        Texture _cubemap;
    };
}
