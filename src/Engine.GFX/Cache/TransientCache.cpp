#include "TransientCache.hpp"

#include "CacheResult.hpp"
#include "ModelBatch.hpp"

using namespace hg::engine::gfx::cache;
using namespace hg::engine::gfx;
using namespace hg;

TransientCache::TransientCache() :
    _mapping() {}

TransientCache::TransientCache(mref<this_type> other_) noexcept :
    _mapping(_STD move(other_._mapping)) {}

TransientCache::~TransientCache() {
    tidy();
}

ref<TransientCache::this_type> TransientCache::operator=(mref<this_type> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _mapping = _STD move(other_._mapping);
    }

    return *this;
}

void TransientCache::tidy() {
    for (auto& entry : _mapping) {
        // TODO: HeliogrimObject::destroy(_STD move(entry.second));
        // Warning: May be unsafe and possible memory leak
        delete entry.second;
    }
    _mapping.clear();
}

CacheResult TransientCache::get(cref<_STD ptrdiff_t> key_, ref<ptr<ModelBatch>> dst_) {
    const auto it { _mapping.find(key_) };

    if (it != _mapping.end()) {
        dst_ = it->second;
        return CacheResult::eHit;
    }

    return CacheResult::eMiss;
}

void TransientCache::insert(cref<_STD ptrdiff_t> key_, mref<ptr<ModelBatch>> obj_) {

    const auto it { _mapping.find(key_) };
    if (it != _mapping.end()) {
        // Warning: May be unsafe and possible memory leak
        delete it->second;
    }

    _mapping.insert_or_assign(key_, _STD move(obj_));
}

void TransientCache::remove(cref<_STD ptrdiff_t> key_) {

    const auto it { _mapping.find(key_) };

    if (it == _mapping.end()) {
        return;
    }

    it->second = nullptr;

    _mapping.erase(it);
}

void TransientCache::remove(cref<_STD ptrdiff_t> key_, const bool tidy_) {

    const auto it { _mapping.find(key_) };

    if (it == _mapping.end()) {
        return;
    }

    // Warning: May be unsafe and possible memory leak
    if (tidy_) {
        delete it->second;
    }
    it->second = nullptr;

    _mapping.erase(it);
}
