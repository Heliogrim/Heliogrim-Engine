#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "BufferLikeObject.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx {
    class SparseBufferView final :
        public InheritMeta<SparseBufferView, BufferLikeObject> {
    public:
        friend class SparseBuffer;

    public:
        using this_type = SparseBufferView;

    protected:
        SparseBufferView(
            non_owning_rptr<SparseBuffer> owner_,
            mref<Vector<non_owning_rptr<SparseBufferPage>>> pages_,
            const u64 offset_,
            const u64 size_
        );

    public:
        SparseBufferView(cref<this_type>) = delete;

        SparseBufferView(mref<this_type>) noexcept = delete;

    public:
        ~SparseBufferView() override;

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;

    private:
        non_owning_rptr<SparseBuffer> _owner;

    public:
        [[nodiscard]] const non_owning_rptr<const SparseBuffer> owner() const noexcept;

    private:
        Vector<non_owning_rptr<SparseBufferPage>> _pages;

    public:
        // TODO: Should be protected / private ...
        [[nodiscard]] cref<Vector<non_owning_rptr<SparseBufferPage>>> pages() const noexcept;

    private:
        u64 _offset;
        u64 _size;

    public:
        [[nodiscard]] u64 offset() const noexcept;

        [[nodiscard]] u64 size() const noexcept;
    };
}
