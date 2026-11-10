#pragma once

#include "BufferFactory.hpp"

namespace hg::engine::gfx {
    class VkBufferFactory final :
        public BufferFactory {
    public:
        static nmpt<VkBufferFactory> make(cref<sptr<Device>> device_);

    private:
        VkBufferFactory(cref<sptr<Device>> device_);

    public:
        ~VkBufferFactory() noexcept override;

    public:
        [[nodiscard]] Buffer build(
            mref<BufferBuildPayload> payload_
        ) const override;
    };
}
