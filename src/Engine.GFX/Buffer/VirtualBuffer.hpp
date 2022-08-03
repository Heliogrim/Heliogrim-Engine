#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "../Device/Device.hpp"
#include "VirtualBufferPage.hpp"
#include "__fwd.hpp"

namespace ember::engine::gfx {

    /**
     * TODO: Add support for software sparse buffer using underlying memory pages
     *  TODO: Each page has a defined size which might be mappable to any left memory sequence
     *  TODO: If hardware/driver doesn't support sparse technology, switch to software solution
     *  TODO: Software solution for sparse set requires indirection buffer to handle sequences
     */

    class VirtualBuffer {
    public:
        using this_type = VirtualBuffer;

    public:
        VirtualBuffer() noexcept;

        VirtualBuffer(mref<uptr<VirtualMemory>> memory_, cref<vk::Buffer> buffer_,
            cref<vk::BufferUsageFlags> usageFlags_) noexcept;

        VirtualBuffer(cref<this_type>) = delete;

        VirtualBuffer(mref<this_type> other_) noexcept;

        ~VirtualBuffer();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type> other_) noexcept;

    private:
        /**
         * Will free backing resources and destroy buffer object
         */
        void tidy();

    private:
        uptr<VirtualMemory> _memory;

    public:
        [[nodiscard]] const non_owning_rptr<const VirtualMemory> memory() const noexcept;

        [[nodiscard]] const non_owning_rptr<VirtualMemory> memory() noexcept;

    private:
        Vector<ptr<VirtualBufferPage>> _pages;

    public:
        [[nodiscard]] u64 size() const noexcept;

        [[nodiscard]] u64 residentialSize() const noexcept;

    private:
        vk::Buffer _vkBuffer;
        vk::BufferUsageFlags _vkBufferUsageFlags;

    public:
        [[nodiscard]] cref<vk::Buffer> vkBuffer() const noexcept;

        [[nodiscard]] cref<vk::BufferUsageFlags> vkBufferUsageFlags() const noexcept;

    private:
        // TODO:
        bool _changed;

    public:
        /**
         * Add a new page to this virtual buffer
         *
         * @author Julius
         * @date 06.01.2022
         *
         * @param size_ The size of bytes of the resource to store at the new page.
         * @param offset_ The range from the start where to insert the resource in bytes.
         *
         * @returns A pointer to the newly created virtual buffer page or nullptr.
         */
        non_owning_rptr<VirtualBufferPage> addPage(const u64 size_, const u64 offset_);

        [[nodiscard]] cref<Vector<ptr<VirtualBufferPage>>> pages() const noexcept;

    private:
        void assureTiledPages(
            const u64 offset_,
            const u64 size_
        );

        void selectPages(
            const u64 offset_,
            const u64 size_,
            _Inout_ ref<Vector<non_owning_rptr<VirtualBufferPage>>> pages_
        );

    public:
        /**
         * Create an sub-resource view of this virtual buffer
         *
         * @author Julius
         * @datte 03.08.20222
         *
         * @returns A unique pointer to the created view
         */
        [[nodiscard]] uptr<VirtualBufferView> makeView(
            const u64 offset_,
            const u64 size_
        );

    private:
        Vector<vk::SparseMemoryBind> _bindings;
        vk::SparseBufferMemoryBindInfo _bindData;

    public:
        void updateBindingData();

    public:
        void enqueueBinding(const ptr<CommandQueue> queue_);

        void enqueueBinding(const ptr<CommandQueue> queue_, cref<Vector<vk::Semaphore>> waits_,
            cref<Vector<vk::Semaphore>> signals_);

        [[deprecated]] void enqueueBindingSync(const ptr<CommandQueue> queue_);
    };
}
