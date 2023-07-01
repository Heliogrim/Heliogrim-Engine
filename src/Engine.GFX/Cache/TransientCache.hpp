#pragma once
#include <algorithm>

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx::cache {
    class TransientCache {
    public:
        using this_type = TransientCache;

    public:
        TransientCache();

        TransientCache(cref<this_type>) = delete;

        TransientCache(mref<this_type> other_) noexcept;

        ~TransientCache();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type> other_) noexcept;

    public:
        void tidy();

    private:
        DenseMap<_STD ptrdiff_t, ptr<ModelBatch>> _mapping;

    public:
        [[nodiscard]] CacheResult get(cref<_STD ptrdiff_t> key_, ref<ptr<ModelBatch>> dst_);

        void insert(cref<_STD ptrdiff_t> key_, mref<ptr<ModelBatch>> obj_);

        void remove(cref<_STD ptrdiff_t> key_);

        void remove(cref<_STD ptrdiff_t> key_, const bool tidy_);
    };
}
