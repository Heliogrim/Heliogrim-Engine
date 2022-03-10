#pragma once

#include "../vkinc.hpp"
#include "../Geometry/Vertex.hpp"
#include "../Memory/AllocatedMemory.hpp"

namespace ember::engine::gfx {

    class Buffer {
    public:
        using this_type = Buffer;

        using value_type = Buffer;
        using reference_type = Buffer&;
        using const_reference_type = const Buffer&;

    public:
        ptr<AllocatedMemory> memory;

    public:
        vk::Buffer buffer;
        vk::Device device;
        u64 size = 0;

        vk::BufferUsageFlags usageFlags;

        /**
         * Binds
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  size_ (Optional) The size.
         * @param  offset_ (Optional) The offset.
         */
        void bind(const u64 size_ = VK_WHOLE_SIZE, const u64 offset_ = 0) {
            device.bindBufferMemory(buffer, memory->vkMemory, offset_);
        }

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 20.11.2020
         */
        void destroy() {
            if (memory->mapping) {
                unmap();
            }

            if (buffer) {
                device.destroyBuffer(buffer);
            }

            if (memory) {
                delete memory;
                memory = nullptr;
            }
        }

        /**
         * Flushes this 
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  size_ (Optional) The size.
         * @param  offset_ (Optional) The offset.
         */
        void flush(const u64 size_ = VK_WHOLE_SIZE, const u64 offset_ = 0) {
            memory->flush(size_, offset_);
        }

        /**
         * Maps
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  size_ (Optional) The size.
         * @param  offset_ (Optional) The offset.
         */
        void map(const u64 size_ = VK_WHOLE_SIZE, const u64 offset_ = 0) {
            memory->map(size_, offset_);
        }

        /**
         * Unmaps this 
         *
         * @author Julius
         * @date 20.11.2020
         */
        void unmap() noexcept {
            memory->unmap();
        }

        /**
         * Writes
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  data_ The data.
         * @param  size_ The size.
         */
        void write(const ptr<void> data_, const u64 size_) {
            memory->write(data_, size_);
        }

        /**
         * Writes
         *
         * @tparam ValueType Type of the value type.
         * @param  data_ The data.
         * @param  count_ Number of.
         */
        template <class ValueType>
        void write(const ptr<void> data_, const u32 count_) {
            const size_t size = sizeof(ValueType) * count_;
            const bool unmapped = (memory->mapping ? false : true);

            if (unmapped) {
                map(size);
            }

            memory->write(data_, size);

            if (unmapped) {
                unmap();
            }
        }
    };

    template <typename Ty>
    class TypeBuffer : public Buffer {
    public:
        using value_type = TypeBuffer<Ty>;
        using reference_type = TypeBuffer<Ty>&;
        using const_reference_type = const TypeBuffer<Ty>&;

        [[nodiscard]] u32 count() const {
            return size / sizeof(Ty);
        }
    };

    typedef TypeBuffer<u32> IndexBuffer;
    typedef TypeBuffer<vertex> VertexBuffer;
}
