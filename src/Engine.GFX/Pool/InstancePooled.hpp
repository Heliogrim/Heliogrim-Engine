#pragma once
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Buffer/Buffer.hpp"
#include "../Buffer/BufferFactory.hpp"
#include "../Memory/GlobalPooledAllocator.hpp"
#include "../Memory/VirtualMemory.hpp"

namespace hg::engine::gfx {
	template <typename ValueType_>
	class InstancePooled {
	public:
		using this_type = InstancePooled<ValueType_>;

	public:
		constexpr static auto stride = sizeof(ValueType_);

	public:
		InstancePooled(cref<sptr<Device>> device_) :
			_device(device_),
			_buffer(nullptr),
			_monotonicIndex(0uL),
			_releasedList() {}

		InstancePooled(cref<this_type>) = delete;

		InstancePooled(mref<this_type>) = delete;

		~InstancePooled() = default;

	public:
		void setup(u32 reservedInstances_) {

			::hg::assertd(_buffer == nullptr);

			/**/

			const auto reservedSize = stride * reservedInstances_;
			_buildPayload = BufferBuildPayload {
				.byteSize = reservedSize,
				.byteAlign = alignof(ValueType_),
				.memoryProperties = MemoryProperty::eHostVisible,
				.vkFlags = vk::BufferCreateFlags {},
				.vkUsage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc |
				vk::BufferUsageFlagBits::eTransferDst,
				.vkSharing = vk::SharingMode::eExclusive
			};

			const auto bufferFactor = BufferFactory::get();
			_buffer = make_uptr<TypeBuffer<ValueType_>>(bufferFactor->build(clone(_buildPayload)));
		}

		void destroy() {

			if (not _buffer) {
				return;
			}

			_buffer->destroy();

			_buffer.reset();
			_monotonicIndex = 0uL;
			_releasedList.clear();
		}

	private:
		sptr<Device> _device;

		BufferBuildPayload _buildPayload;
		uptr<TypeBuffer<ValueType_>> _buffer;

		u32 _monotonicIndex;
		Vector<u32> _releasedList;

	private:
		void grow(s64 required_) {
			::hg::todo_panic();
		}

	public:
		struct AcquireResult {
			u32 instanceIndex;
			ref<TypeBuffer<ValueType_>> dataView;
		};

		[[nodiscard]] AcquireResult acquire() {

			if (not _releasedList.empty()) {

				const auto index = _releasedList.back();
				_releasedList.pop_back();

				return { index, *_buffer };
			}

			/**/

			auto index = _monotonicIndex++;

			if (_buffer->size < index * stride) {
				grow(-1LL);
			}

			return { index, *_buffer };
		}

		[[nodiscard]] AcquireResult acquire(const u32 instance_) {

			if (not _releasedList.empty()) {

				auto iter = _releasedList.rbegin();
				for (; iter != _releasedList.rend(); ++iter) {
					if ((*iter) == instance_) {
						break;
					}
				}

				if (iter != _releasedList.rend()) {
					_releasedList.erase(iter.base() - 1uL);
					return { instance_, *_buffer };
				}
			}

			/**/

			_monotonicIndex = MAX(_monotonicIndex, instance_);

			if (_buffer->memorySize() < instance_ * stride) {
				grow(instance_ * stride);
			}

			return { instance_, *_buffer };
		}

		void release(u32 instanceIndex_) {
			_releasedList.emplace_back(instanceIndex_);
		}

	public:
		[[nodiscard]] nmpt<TypeBuffer<ValueType_>> getPoolView() const {
			return _buffer.get();
		}
	};
}
