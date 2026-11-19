#pragma once

#include <Engine.Common/__macro.hpp>

#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::cache {
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
        DenseMap<std::ptrdiff_t, ptr<ModelBatch>> _mapping;

    public:
        [[nodiscard]] CacheResult get(cref<std::ptrdiff_t> key_, ref<ptr<ModelBatch>> dst_);

        void insert(cref<std::ptrdiff_t> key_, mref<ptr<ModelBatch>> obj_);

        void remove(cref<std::ptrdiff_t> key_);

        void remove(cref<std::ptrdiff_t> key_, const bool tidy_);
    };
}
