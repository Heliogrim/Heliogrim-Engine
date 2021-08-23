#pragma once

#include "../Geometry/Vertex.hpp"
#include "../vkinc.hpp"

namespace ember::engine::gfx {
    class Buffer {
    public:
        using value_type = Buffer;
        using reference_type = Buffer&;
        using const_reference_type = const Buffer&;

        vk::Buffer buffer;
        vk::Device device;
        vk::DeviceMemory memory;
        u64 size = 0;
        u64 align = 0;

        void* mapped = nullptr;

        vk::BufferUsageFlags usageFlags;
        vk::MemoryPropertyFlags memoryFlags;

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
            device.bindBufferMemory(buffer, memory, offset_);
        }

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 20.11.2020
         */
        void destroy() {
            unmap();

            if (buffer) {
                device.destroyBuffer(buffer);
            }

            if (memory) {
                device.freeMemory(memory);
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
            vk::MappedMemoryRange range {
                memory,
                offset_,
                size_
            };

            device.flushMappedMemoryRanges(1, &range);
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
            mapped = device.mapMemory(memory, offset_, size_, vk::MemoryMapFlags());
        }

        /**
         * Unmaps this 
         *
         * @author Julius
         * @date 20.11.2020
         */
        void unmap() noexcept {
            if (!mapped) {
                return;
            }

            device.unmapMemory(memory);
            mapped = nullptr;
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
        void write(const void* data_, const u64 size_) {
            memcpy(mapped, data_, static_cast<size_t>(size_));
        }

        /**
         * Writes
         *
         * @tparam ValueType Type of the value type.
         * @param  data_ The data.
         * @param  count_ Number of.
         */
        template <class ValueType>
        void write(const void* data_, const u32 count_) {
            const size_t size = sizeof(ValueType) * count_;
            const bool unmapped = (mapped ? false : true);

            if (unmapped) {
                map(size);
            }

            memcpy(mapped, data_, size);

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
