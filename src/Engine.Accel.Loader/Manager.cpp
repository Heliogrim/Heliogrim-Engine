#include "Manager.hpp"

#include "Effect/EffectLoader.hpp"

using namespace hg::engine::accel;
using namespace hg;

void engine::accel::register_loader(
    ref<resource::LoaderManager> manager_,
    mref<smr<GlobalStorage>> globalStorage_
) {
    auto el = make_sptr<loader::EffectLoader>(std::move(globalStorage_), manager_.sharedSourceLoader());
    manager_.registerLoader<assets::AccelEffect, EffectResource>(std::move(el));
}

void engine::accel::unregister_loader(
    ref<resource::LoaderManager> manager_,
    cref<smr<GlobalStorage>> globalStorage_
) {
    manager_.unregisterLoader<assets::AccelEffect>();
}
