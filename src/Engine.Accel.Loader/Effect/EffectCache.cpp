#include "EffectCache.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>
#include <utility>

using namespace hg::engine::resource::loader;
using namespace hg::engine::accel::loader;
using namespace hg::engine::accel;
using namespace hg;

EffectCache::EffectCache(mref<smr<GlobalStorage>> storage_) :
    Cache(),
    _storage(_STD move(storage_)) {}

EffectCache::~EffectCache() = default;

EffectCache::response_type::type EffectCache::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    const asset_guid guid = request_->get_guid();

    if (_storage->hasAccelEffect(guid)) {
        auto stored = _storage->getAccelEffect(guid);
        return make_smr<resource::UniqueResource<AccelEffectWrapper>>(_STD move(stored));
    }

    auto prevResponse = next_(_STD move(request_), _STD move(options_));

    if (not prevResponse.empty()) {
        const auto guard = prevResponse->acquire(resource::ResourceUsageFlag::eRead);
        const auto& effect = guard.imm()->instance;
        _storage->insertAccelEffect(calcStorageHash(*effect), clone(effect));
    }

    return prevResponse;
}
