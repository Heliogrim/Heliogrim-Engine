#include "GlobalResourceCache.hpp"

#include "Engine.GFX/Geometry/Vertex.hpp"
#include "Engine.GFX/Memory/AllocationResult.hpp"
#include "../Texture/TextureFactory.hpp"

#include <Engine.GFX.Loader/Texture/Traits.hpp>
#include <Engine.GFX.Loader/Geometry/Traits.hpp>

using namespace hg::engine::gfx::cache;
using namespace hg::engine::gfx;
using namespace hg;

GlobalResourceCache::GlobalResourceCache(cref<sptr<Device>> device_) :
    _device(device_) {}

GlobalResourceCache::~GlobalResourceCache() {
    tidy();
};

void GlobalResourceCache::tidy() {
    // TODO: Check whether we need to release all resources manually
    _mapped.clear();
}

cref<sptr<Device>> GlobalResourceCache::device() const noexcept {
    return _device;
}

bool GlobalResourceCache::contains(const non_owning_rptr<const assets::Asset> asset_) const noexcept {
    return _mapped.find(asset_->get_guid()) != _mapped.end();
}

GlobalResourceCache::query_result_type GlobalResourceCache::query(
    const non_owning_rptr<const assets::Asset> asset_
) const noexcept {
    return query(asset_->get_guid());
}

GlobalResourceCache::query_result_type GlobalResourceCache::query(cref<asset_guid> guid_) const noexcept {

    const auto it = _mapped.find(guid_);
    if (it != _mapped.end()) {
        return { QueryResultType::eHit, cache_value_type { it->second } };
    }

    return { QueryResultType::eMiss };
}

GlobalResourceCache::store_result_type GlobalResourceCache::store(
    cref<asset_guid> guid_,
    mref<smr<resource::ResourceBase>> resource_
) {
    /**
     * Store must be unique
     */
    const auto it = _mapped.find(guid_);
    if (it != _mapped.end()) {
        return { StoreResultType::eFail, cache_value_type { it->second } };
    }

    // TODO: Can be removed?
    // auto managed { resource_->acquire(resource::ResourceUsageFlag::eAll) };
    // managed.release();

    /**
     * Store hooked resource
     */
    _mapped.insert_or_assign(guid_, std::move(resource_));
    return { StoreResultType::eSuccess };
}

bool GlobalResourceCache::remove(cref<asset_guid> guid_) {
    return _mapped.erase(guid_);
}

bool GlobalResourceCache::remove(
    cref<asset_guid> guid_,
    _Out_ ref<smr<resource::ResourceBase>> resource_
) {

    const auto it = _mapped.find(guid_);
    if (it == _mapped.end()) {
        return false;
    }

    resource_ = std::move(it->second);
    _mapped.erase(it);

    return true;
}

void GlobalResourceCache::clear() {
    tidy();
}
