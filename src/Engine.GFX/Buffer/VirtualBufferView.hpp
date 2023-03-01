#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx {
    class VirtualBufferView final {
    public:
        friend class VirtualBuffer;

    public:
        using this_type = VirtualBufferView;

    protected:
        VirtualBufferView(
            non_owning_rptr<VirtualBuffer> owner_,
            mref<Vector<non_owning_rptr<VirtualBufferPage>>> pages_,
            const u64 offset_,
            const u64 size_
        );

    public:
        VirtualBufferView(cref<this_type>) = delete;

        VirtualBufferView(mref<this_type>) noexcept = delete;

    public:
        ~VirtualBufferView();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;

    private:
        non_owning_rptr<VirtualBuffer> _owner;

    public:
        [[nodiscard]] const non_owning_rptr<const VirtualBuffer> owner() const noexcept;

    private:
        Vector<non_owning_rptr<VirtualBufferPage>> _pages;

    public:
        // TODO: Should be protected / private ...
        [[nodiscard]] cref<Vector<non_owning_rptr<VirtualBufferPage>>> pages() const noexcept;

    private:
        u64 _offset;
        u64 _size;

    public:
        [[nodiscard]] u64 offset() const noexcept;

        [[nodiscard]] u64 size() const noexcept;
    };
}
