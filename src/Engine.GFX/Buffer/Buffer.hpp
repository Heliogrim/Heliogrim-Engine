#pragma once

#include <utility>
#include <Engine.Common/Move.hpp>

#include "BufferObject.hpp"
#include "../vkinc.hpp"
#include "../Geometry/Vertex.hpp"
#include "../Memory/AllocatedMemory.hpp"

namespace hg::engine::gfx {
	class Buffer :
		public InheritMeta<Buffer, BufferObject> {
	public:
		using this_type = Buffer;

		using value_type = Buffer;
		using reference_type = Buffer&;
		using const_reference_type = const Buffer&;

	public:
		constexpr Buffer() noexcept = default;

		Buffer(mref<this_type> other_) noexcept :
			memory(std::exchange(other_.memory, nullptr)),
			buffer(std::exchange(other_.buffer, nullptr)),
			device(other_.device),
			size(other_.size),
			usageFlags(other_.usageFlags) {}

		~Buffer() noexcept override = default;

	public:
		ref<this_type> operator=(mref<this_type> other_) noexcept {
			if (this != std::addressof(other_)) {
				memory = std::exchange(other_.memory, nullptr);
				buffer = std::exchange(other_.buffer, nullptr);
				device = other_.device;
				size = other_.size;
				usageFlags = other_.usageFlags;
			}
			return *this;
		}

	public:
		uptr<memory::AllocatedMemory> memory = nullptr;

	public:
		vk::Buffer buffer = nullptr;
		vk::Device device = nullptr;
		u64 size = 0;

		vk::BufferUsageFlags usageFlags = vk::BufferUsageFlags {};

		/**
		 * Binds
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @param  size_ (Optional) The size.
		 * @param  offset_ (Optional) The offset.
		 */
		void bind(const u64 size_ = VK_WHOLE_SIZE, const u64 offset_ = 0);

		/**
		 * Destroys this
		 *
		 * @author Julius
		 * @date 20.11.2020
		 */
		void destroy();

		/**
		 * Flushes this
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @param  size_ (Optional) The size.
		 * @param  offset_ (Optional) The offset.
		 */
		void flushAligned(const u64 size_ = VK_WHOLE_SIZE, const u64 offset_ = 0);

		/**
		 * Maps
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @param  size_ (Optional) The size.
		 * @param  offset_ (Optional) The offset.
		 */
		void map(const u64 size_ = VK_WHOLE_SIZE, const u64 offset_ = 0);

		/**
		 * Map the buffer memory aligned (non coherent atom size) to host
		 *
		 * @author Julius
		 * @date 01.08.20222
		 *
		 * @param  size_ (Optional) The size.
		 * @param  offset_ (Optional) The offset.
		 */
		void mapAligned(const u64 size_ = VK_WHOLE_SIZE, const u64 offset_ = 0);

		/**
		 * Unmaps this
		 *
		 * @author Julius
		 * @date 20.11.2020
		 */
		void unmap() noexcept;

		/**
		 * Writes
		 *
		 * @author Julius
		 * @date 20.11.2020
		 *
		 * @param  data_ The data.
		 * @param  size_ The size.
		 */
		void write(const ptr<void> data_, const u64 size_);

		/**
		 * Writes
		 *
		 * // TODO: Make two overloads; One for coherent memory, one for flushable memory
		 *
		 * @tparam ValueType Type of the value type.
		 * @param  data_ The data.
		 * @param  count_ Number of.
		 */
		template <class ValueType>
		void write(const ptr<const void> data_, const u32 count_) {
			const size_t dataSize = sizeof(ValueType) * count_;
			const bool unmapped = (memory->mapping ? false : true);

			if (unmapped) {
				map(dataSize);
			}

			memory->write(data_, dataSize);

			if (unmapped) {
				unmap();
			}
		}
	};

	template <typename Ty>
	class TypeBuffer : public Buffer {
	public:
		using this_type = TypeBuffer;

	public:
		TypeBuffer() noexcept = default;

		TypeBuffer(mref<Buffer> buffer_) noexcept :
			Buffer(::hg::move(buffer_)) {}

		TypeBuffer(ref<const this_type>) = delete;

		TypeBuffer(mref<this_type> other_) noexcept :
			Buffer(::hg::move(other_)) {}

		~TypeBuffer() noexcept override = default;

	public:
		ref<this_type> operator=(mref<this_type> other_) noexcept = default;

		ref<this_type> operator=(mref<Buffer> other_) noexcept {

			static_assert(sizeof(this_type) == sizeof(Buffer));
			static_assert(alignof(this_type) == alignof(Buffer));

			static_cast<ref<Buffer>>(*this) = std::move(other_);
			return *this;
		}

	public:
		[[nodiscard]] u32 count() const {
			return size / sizeof(Ty);
		}
	};

	typedef TypeBuffer<u32> IndexBuffer;
	typedef TypeBuffer<vertex> VertexBuffer;
}
