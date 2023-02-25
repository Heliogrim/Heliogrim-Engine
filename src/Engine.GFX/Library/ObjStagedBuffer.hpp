#pragma once

#include "Inc.hpp"
#include "Device.hpp"
#include "StagedBuffer.hpp"

namespace clockwork::__internal::gfx::types {
    template <typename Ty>
    class ObjStagedBuffer :
        public StagedBuffer {
    public:
        ObjStagedBuffer() :
            StagedBuffer() { };

        void setupDescriptor() override {
            const size64_t size = sizeof(Ty) * _instanceCount;
            _buffer.setupDescriptor(size);
        }

    protected:
        ObjStagedBuffer(
            const clockwork::__internal::gfx::types::__Device& device_,
            const Buffer& stage_,
            const Buffer& dest_,
            const size_t instanceCount_
        ) :
            StagedBuffer(device_, stage_, dest_),
            _instanceCount(instanceCount_) { }

    private:
        size_t _instanceCount = 0;

    public:
        /**
         * Constructor function to call
         */
        static ObjStagedBuffer<Ty> make(
            const clockwork::__internal::gfx::types::__Device& device_,
            const BufferUsageFlags& usage_,
            const size_t instanceCount_ = 1
        ) {
            if (instanceCount_ < 1)
                throw std::runtime_error("Can not create a staged object bound buffer with instance count of zero.");

            const size64_t size = sizeof(Ty) * instanceCount_;

            Buffer stage = device_->createBuffer(
                BufferUsageFlagBits::eTransferDst | BufferUsageFlagBits::eTransferSrc,
                MemoryPropertyFlagBits::eHostCoherent | MemoryPropertyFlagBits::eHostVisible,
                size
            );

            Buffer dest = device_->createBuffer(
                usage_ | BufferUsageFlagBits::eTransferDst,
                MemoryPropertyFlagBits::eDeviceLocal,
                size
            );

            stage.bind();
            dest.bind();

            return ObjStagedBuffer(device_, stage, dest, instanceCount_);
        }
    };
}
