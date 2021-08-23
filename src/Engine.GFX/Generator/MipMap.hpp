#pragma once

#include "../Command/CommandQueue.hpp"
#include "../Device/Device.hpp"
#include "../Texture/Texture.hpp"

namespace ember::engine::gfx::generator {
    class MipMapGenerator {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 25.11.2020
         */
        MipMapGenerator() = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 25.11.2020
         */
        ~MipMapGenerator() noexcept = default;

        /**
         * Generates
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @param [in,out] texture_ The texture.
         * @param 		   device_ The device.
         * @param [in]	   queue_ The queue.
         */
        void generate(IN OUT Texture& texture_, cref<sptr<Device>> device_, IN CommandQueue& queue_);

        /**
         * Calculates the mip level
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @param  extent_ The extent.
         *
         * @returns The calculated mip level.
         */
        [[nodiscard]] u32 calcMipLevel(const u32 extent_) const noexcept;

    private:
    };
}
