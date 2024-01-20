#pragma once
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../Memory/VirtualMemory.hpp"
#include "../Buffer/SparseBuffer.hpp"
#include "../Memory/GlobalPooledAllocator.hpp"
#include "../Buffer/SparseBufferView.hpp"
#include "Engine.GFX/Command/CommandBuffer.hpp"
#include "Engine.GFX/Command/CommandPool.hpp"
#include "Engine.GFX/Command/CommandQueue.hpp"

namespace hg::engine::gfx {
    template <typename ValueType_>
    class RangePooled {
    public:
        using this_type = RangePooled<ValueType_>;

    public:
        RangePooled(cref<sptr<Device>> device_) :
            _device(device_),
            _allocator(_device->allocator()),
            _buffer(nullptr),
            _monotonicOffset(0uL),
            _releasedList() {}

        RangePooled(cref<this_type>) = delete;

        RangePooled(mref<this_type>) = delete;

        ~RangePooled() = default;

    private:
    public:
        vk::BufferCreateFlags _vkBufferCreate;
        vk::BufferUsageFlags _vkBufferUsage;

    public:
        void setup(u64 reserved_) {

            assert(_buffer == nullptr);

            /**/

            vk::BufferCreateInfo bci {
                vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency | _vkBufferCreate,
                reserved_,
                _vkBufferUsage,
                vk::SharingMode::eExclusive,
                0ui32,
                nullptr,
                nullptr
            };

            auto vkBuffer = _device->vkDevice().createBuffer(bci);

            const auto req = _device->vkDevice().getBufferMemoryRequirements(vkBuffer);
            const auto layout = memory::MemoryLayout {
                req.alignment,
                MemoryProperty::eDeviceLocal,
                req.memoryTypeBits
            };

            auto memory = make_uptr<VirtualMemory>(_allocator, layout, req.size);
            auto buffer = make_uptr<SparseBuffer>(_STD move(memory), reserved_, vkBuffer, bci.usage);

            buffer->updateBindingData();

            #pragma warning(push)
            #pragma warning(disable : 4996)
            buffer->enqueueBindingSync(_device->graphicsQueue());
            #pragma warning(pop)

            _buffer = _STD move(buffer);
        }

        void destroy() {

            if (not _buffer) {
                return;
            }

            _buffer.reset();
            _monotonicOffset = 0uL;
            _releasedList.clear();
        }

    private:
        sptr<Device> _device;
        nmpt<memory::GlobalPooledAllocator> _allocator;

    private:
        uptr<SparseBuffer> _buffer;

        u64 _monotonicOffset;
        Vector<_STD pair<u64, u64>> _releasedList;

    private:
        void grow(u64 required_) {

            const auto alignment = _buffer->memory()->layout().align;

            const auto prevSize = _buffer->memorySize();
            const auto nextSize = (required_ / alignment) * alignment + ((required_ % alignment) ? alignment : 0uLL);

            /**/

            vk::BufferCreateInfo bci {
                vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency | _vkBufferCreate,
                nextSize,
                _vkBufferUsage,
                vk::SharingMode::eExclusive,
                0ui32,
                nullptr,
                nullptr
            };

            auto vkBuffer = _device->vkDevice().createBuffer(bci);

            const auto req = _device->vkDevice().getBufferMemoryRequirements(vkBuffer);
            const auto layout = memory::MemoryLayout {
                req.alignment,
                MemoryProperty::eDeviceLocal,
                req.memoryTypeBits
            };

            auto memory = make_uptr<VirtualMemory>(_allocator, layout, req.size);
            auto buffer = make_uptr<SparseBuffer>(_STD move(memory), nextSize, vkBuffer, bci.usage);

            const auto requiredPages = (nextSize / req.alignment) + ((nextSize % req.alignment) ? 1uLL : 0uLL);

            for (u64 page = 0uLL; page < requiredPages; ++page) {
                auto vbp = buffer->addPage(req.alignment, req.alignment * page);
                [[maybe_unused]] const auto loadResult = vbp->load();
                assert(loadResult);
            }

            buffer->updateBindingData();

            #pragma warning(push)
            #pragma warning(disable : 4996)
            buffer->enqueueBindingSync(_device->graphicsQueue());
            #pragma warning(pop)

            /**/

            const auto cmdPool = _device->transferQueue()->pool();
            cmdPool->lck().acquire();
            auto cmd = cmdPool->make();

            cmd.begin();
            _STD array<vk::BufferMemoryBarrier, 2uLL> before {
                vk::BufferMemoryBarrier {
                    vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eTransferRead, 0uL, 0uL, _buffer->vkBuffer(),
                    0uL, prevSize
                },
                vk::BufferMemoryBarrier {
                    vk::AccessFlags {}, vk::AccessFlagBits::eTransferWrite, 0uL, 0uL, buffer->vkBuffer(), 0uL, nextSize
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

            _STD array<vk::BufferMemoryBarrier, 2uLL> after {
                vk::BufferMemoryBarrier {
                    vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eShaderRead, 0uL, 0uL, _buffer->vkBuffer(),
                    0uL, prevSize
                },
                vk::BufferMemoryBarrier {
                    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, 0uL, 0uL, buffer->vkBuffer(),
                    0uL, nextSize
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

            _buffer = _STD move(buffer);
        }

    public:
        [[nodiscard]] uptr<SparseBufferView> acquire(u64 requestSize_) {

            auto mark = _releasedList.begin();
            for (auto it = _releasedList.begin(); it != _releasedList.end(); ++it) {

                const auto range = it->second - it->first;
                if (range < requestSize_) {
                    continue;
                }

                const auto markRange = mark->second - mark->first;
                if (range > markRange) {
                    continue;
                }

                mark = it;
            }

            if (mark != _releasedList.end() && (mark->second - mark->first) >= requestSize_) {
                const auto range = *mark;
                _releasedList.erase(mark);
                return _buffer->makeView(range.first, (range.second - range.first));
            }

            /**/

            _monotonicOffset += requestSize_;
            const auto range = _STD pair(_monotonicOffset - requestSize_, _monotonicOffset);

            assert(_buffer->memorySize() > _monotonicOffset);

            /**/

            const auto pageAlign = _buffer->memory()->layout().align;
            const auto requiredPages = (requestSize_ / pageAlign)
                + ((requestSize_ % pageAlign) ? 1uLL : 0uLL);

            const u64 basePage = _buffer->pages().size();
            for (u64 page = 0uLL; page < requiredPages; ++page) {

                auto vbp = _buffer->addPage(pageAlign, (basePage + page) * pageAlign);
                [[maybe_unused]] const auto loadResult = vbp->load();
                assert(loadResult);
            }

            _buffer->updateBindingData();

            #pragma warning(push)
            #pragma warning(disable : 4996)
            _buffer->enqueueBindingSync(_device->graphicsQueue());
            #pragma warning(pop)

            /**/

            return _buffer->makeView(range.first, requestSize_);
        }

        void release(mref<uptr<SparseBufferView>> view_) {
            _releasedList.emplace_back(view_->offset(), view_->offset() + view_->size());
            view_.reset();
        }
    };
}
