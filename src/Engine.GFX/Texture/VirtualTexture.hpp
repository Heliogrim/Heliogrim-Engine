#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include "Texture.hpp"
#include "../Device/Device.hpp"

namespace ember::engine::gfx {

    class VirtualTexturePage final {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 20.11.2020
         */
        VirtualTexturePage();

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.11.2020
         */
        ~VirtualTexturePage() noexcept;

        /**
         * Check whether resident this 
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool resident() const noexcept;

        /**
         * Allocates virtual page at given device
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  device_ The device.
         */
        void allocate(cref<sptr<Device>> device_);

        /**
         * Releases this at given device
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  device_ The device.
         */
        void release(cref<sptr<Device>> device_);

    private:
        math::uivec3 _offset;
        math::uivec3 _extent;

        u32 _mipLevel;
        u32 _layer;
        u32 _idx;
    };

    class VirtualTexture :
        public Texture {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 20.11.2020
         */
        VirtualTexture();

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.11.2020
         */
        ~VirtualTexture() noexcept;

    private:
        vector<VirtualTexturePage> _pages;
    };
}
