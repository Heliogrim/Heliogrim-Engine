#pragma once

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../Buffer/SparseBuffer.hpp"
#include "../Buffer/SparseBufferView.hpp"
#include "../Command/CommandBuffer.hpp"
#include "../Command/CommandPool.hpp"
#include "../Command/CommandQueue.hpp"
#include "../Memory/GlobalPooledAllocator.hpp"
#include "../Memory/VirtualMemory.hpp"

namespace hg::engine::gfx {
	template <typename ValueType_>
	class SparseInstancePooled {
	public:
		using this_type = SparseInstancePooled<ValueType_>;

	public:
		constexpr static auto stride = sizeof(ValueType_);

	public:
		SparseInstancePooled(cref<sptr<Device>> device_) :
			_device(device_),
			_allocator(_device->allocator()),
			_buffer(nullptr),
			_monotonicIndex(0uL),
			_releasedList() {}

		SparseInstancePooled(cref<this_type>) = delete;

		SparseInstancePooled(mref<this_type>) = delete;

		~SparseInstancePooled() = default;

	public:
		void setup(u32 reservedInstances_) {

			assert(_buffer == nullptr);

			/**/

			const auto reservedSize = stride * reservedInstances_;

			vk::BufferCreateInfo bci {
				vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency,
				reservedSize,
				vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc |
				vk::BufferUsageFlagBits::eTransferDst,
				vk::SharingMode::eExclusive,
				0uL,
				nullptr,
				nullptr
			};

			auto vkBuffer = _device->vkDevice().createBuffer(bci);

			const auto req = _device->vkDevice().getBufferMemoryRequirements(vkBuffer);
			const auto layout = memory::MemoryLayout {
				req.alignment,
				MemoryProperty::eHostVisible,
				req.memoryTypeBits
			};

			auto memory = make_uptr<VirtualMemory>(_allocator, layout, req.size);
			auto buffer = make_uptr<SparseBuffer>(std::move(memory), reservedSize, vkBuffer, bci.usage);

			const auto requiredPages = (reservedSize / req.alignment) + ((reservedSize % req.alignment) ? 1uLL : 0uLL);

			for (u64 page = 0uLL; page < requiredPages; ++page) {
				auto vbp = buffer->addPage(req.alignment, req.alignment * page);
				[[maybe_unused]] const auto loadResult = vbp->load();
				assert(loadResult);
			}

			buffer->updateBindingData();
			buffer->enqueueBindingSync(_device->graphicsQueue());

			_buffer = std::move(buffer);
		}

		void destroy() {

			if (not _buffer) {
				return;
			}

			_buffer.reset();
			_monotonicIndex = 0uL;
			_releasedList.clear();
		}

	private:
		sptr<Device> _device;
		nmpt<memory::GlobalPooledAllocator> _allocator;

	private:
		uptr<SparseBuffer> _buffer;

		u32 _monotonicIndex;
		Vector<u32> _releasedList;

	private:
		void grow(s64 required_) {

			const auto prevSize = _buffer->memorySize();

			const auto nextSize = (required_ > 0) ?
				MAX(required_, _buffer->memorySize()) :
				_buffer->memorySize() + 128uLL * stride;
			const auto dr = lldiv(nextSize, 128uLL);
			const auto nextSizeAligned = dr.quot + (dr.rem > 0 ? 1uLL : 0uLL);

			/**/

			vk::BufferCreateInfo bci {
				vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency,
				nextSizeAligned,
				vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc |
				vk::BufferUsageFlagBits::eTransferDst,
				vk::SharingMode::eExclusive,
				0uL,
				nullptr,
				nullptr
			};

			auto vkBuffer = _device->vkDevice().createBuffer(bci);

			const auto req = _device->vkDevice().getBufferMemoryRequirements(vkBuffer);
			const auto layout = memory::MemoryLayout {
				req.alignment,
				MemoryProperty::eHostVisible,
				req.memoryTypeBits
			};

			auto memory = make_uptr<VirtualMemory>(_allocator, layout, req.size);
			auto buffer = make_uptr<SparseBuffer>(std::move(memory), nextSizeAligned, vkBuffer, bci.usage);

			const auto requiredPages = (nextSizeAligned / req.alignment) + ((nextSizeAligned % req.alignment) ?
				1uLL :
				0uLL);

			for (u64 page = 0uLL; page < requiredPages; ++page) {
				auto vbp = buffer->addPage(req.alignment, req.alignment * page);
				[[maybe_unused]] const auto loadResult = vbp->load();
				assert(loadResult);
			}

			buffer->updateBindingData();
			buffer->enqueueBindingSync(_device->graphicsQueue());

			/**/

			const auto cmdPool = _device->transferQueue()->pool();
			cmdPool->lck().acquire();
			auto cmd = cmdPool->make();

			cmd.begin();
			std::array<vk::BufferMemoryBarrier, 2uLL> before {
				vk::BufferMemoryBarrier {
					vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eTransferRead, 0uL, 0uL, _buffer->vkBuffer(),
					0uL, prevSize
				},
				vk::BufferMemoryBarrier {
					vk::AccessFlags {}, vk::AccessFlagBits::eTransferWrite, 0uL, 0uL, buffer->vkBuffer(), 0uL,
					nextSizeAligned
				}
			};
			cmd.vkCommandBuffer().pipelineBarrier(
				vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlagBits::eTransfer,
				vk::DependencyFlagBits::eByRegion,
				0uL,
				nullptr,
				static_cast<u32>(before.size()),
				before.data(),
				0uL,
				nullptr
			);

			const vk::BufferCopy region { 0uL, 0uL, prevSize };
			cmd.vkCommandBuffer().copyBuffer(_buffer->vkBuffer(), buffer->vkBuffer(), 1uL, &region);

			std::array<vk::BufferMemoryBarrier, 2uLL> after {
				vk::BufferMemoryBarrier {
					vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eShaderRead, 0uL, 0uL, _buffer->vkBuffer(),
					0uL, prevSize
				},
				vk::BufferMemoryBarrier {
					vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, 0uL, 0uL, buffer->vkBuffer(),
					0uL, nextSizeAligned
				}
			};
			cmd.vkCommandBuffer().pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eAllCommands,
				vk::DependencyFlagBits::eByRegion,
				0uL,
				nullptr,
				static_cast<u32>(after.size()),
				after.data(),
				0uL,
				nullptr
			);
			cmd.end();

			_device->transferQueue()->submitWait(cmd);

			cmd.release();
			cmdPool->lck().release();

			/**/

			_buffer = std::move(buffer);
		}

	public:
		struct AcquireResult {
			u32 instanceIndex;
			uptr<SparseBufferView> dataView;
		};

		[[nodiscard]] AcquireResult acquire() {

			if (not _releasedList.empty()) {

				const auto index = _releasedList.back();
				_releasedList.pop_back();

				return { index, _buffer->makeView(index * stride, stride) };
			}

			/**/

			auto index = _monotonicIndex++;

			if (_buffer->memorySize() < index * stride) {
				grow(-1LL);
			}

			return { index, _buffer->makeView(index * stride, stride) };
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
					return { instance_, _buffer->makeView(instance_ * stride, stride) };
				}
			}

			/**/

			_monotonicIndex = MAX(_monotonicIndex, instance_);

			if (_buffer->memorySize() < instance_ * stride) {
				grow(instance_ * stride);
			}

			return { instance_, _buffer->makeView(instance_ * stride, stride) };
		}

		void release(u32 instanceIndex_) {
			_releasedList.emplace_back(instanceIndex_);
		}

	public:
		[[nodiscard]] uptr<SparseBufferView> getDataView(const u32 instanceIndex_) const {
			return _buffer->makeView(instanceIndex_ * stride, stride);
		}

		[[nodiscard]] nmpt<SparseBuffer> getPoolView() const {
			return _buffer.get();
		}
	};
}
