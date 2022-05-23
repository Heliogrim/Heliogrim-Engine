#include "LocalResourceCache.hpp"

#include "CacheResult.hpp"
#include "ModelBatch.hpp"

using namespace ember::engine::gfx::cache;
using namespace ember::engine::gfx;
using namespace ember;

LocalResourceCache::LocalResourceCache() :
    _caches(),
    _shifting({ &_caches.front(), &_caches.back() }) {}

LocalResourceCache::LocalResourceCache(mref<this_type> other_) noexcept :
    _caches(_STD move(other_._caches)),
    _shifting({ &_caches.front(), &_caches.back() }) {}

LocalResourceCache::~LocalResourceCache() = default;

ref<LocalResourceCache::this_type> LocalResourceCache::operator=(mref<this_type> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _caches = _STD move(other_._caches);

        if (other_._shifting.front() > other_._shifting.back()) {
            _shifting.front() = &_caches.front();
            _shifting.back() = &_caches.back();
        } else {
            _shifting.front() = &_caches.back();
            _shifting.back() = &_caches.front();
        }
    }

    return *this;
}

void LocalResourceCache::tidy() {
    for (auto& cache : _caches) {
        cache.tidy();
    }
}

CacheResult LocalResourceCache::fetch(cref<_STD ptrdiff_t> key_, _Out_ ref<ptr<ModelBatch>> dst_) {

    /**
     * Check if current cache has object already stored
     */
    if (_shifting.front()->get(key_, dst_) == CacheResult::eHit) {
        return CacheResult::eHit;
    }

    /**
     * If current primary cache missed, check for transient secondary
     */
    if (_shifting.back()->get(key_, dst_) == CacheResult::eHit) {
        store(key_, ptr<ModelBatch> { dst_ });
        _shifting.back()->remove(key_);

        return CacheResult::eHit;
    }

    return CacheResult::eMiss;
}

void LocalResourceCache::store(cref<_STD ptrdiff_t> key_, mref<ptr<ModelBatch>> obj_) {
    _shifting.front()->insert(key_, _STD move(obj_));
}

void LocalResourceCache::shift() {

    /**
     * Swap primary and secondary
     */
    _STD swap(_shifting.front(), _shifting.back());

    /**
     * Erase previous state of primary cache (new secondary can be used to fetch last invocation states)
     */
    _shifting.front()->tidy();
}
