#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"
#include "TransientCache.hpp"

namespace ember::engine::gfx::cache {

    class LocalResourceCache {
    public:
        using this_type = LocalResourceCache;

    public:
        LocalResourceCache();

        LocalResourceCache(cref<this_type>) = delete;

        LocalResourceCache(mref<this_type> other_) noexcept;

        ~LocalResourceCache();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type> other_) noexcept;

    public:
        void tidy();

    private:
        _STD array<TransientCache, 2> _caches;
        _STD array<ptr<TransientCache>, 2> _shifting;

    public:
        [[nodiscard]] CacheResult fetch(cref<_STD ptrdiff_t> key_, _Out_ ref<ptr<ModelBatch>> dst_);

        void store(cref<_STD ptrdiff_t> key_, mref<ptr<ModelBatch>> obj_);

    public:
        void shift();

    };
}
