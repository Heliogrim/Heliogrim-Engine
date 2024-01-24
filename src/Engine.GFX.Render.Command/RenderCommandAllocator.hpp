#pragma once
#include <Engine.Common/Collection/InlineAutoArray.hpp>

#include "RenderCommandAllocatorBase.hpp"

namespace hg::engine::render::cmd {
    class RenderCommandAllocator :
        public RenderCommandAllocatorBase {
    public:
        using this_type = RenderCommandAllocator;

        using memory_type = std::pmr::memory_resource;

        constexpr static size_type initial_memory_size = 1024uLL;

    protected:
        RenderCommandAllocator() noexcept;

        RenderCommandAllocator(mref<this_type> other_) noexcept;

    public:
        ~RenderCommandAllocator() noexcept override;

    public:
        ref<this_type> operator=(mref<this_type>) = delete;

        ref<this_type> operator=(cref<this_type>) = delete;

    protected:
        InlineAutoArray<uptr<memory_type>> _memory;

    public:
        void release();

    public:
        [[nodiscard]] void* allocateUnique(const size_type size_) override;

        [[nodiscard]] bool deallocateUnique(mref<void*> memory_) override;
    };
}
