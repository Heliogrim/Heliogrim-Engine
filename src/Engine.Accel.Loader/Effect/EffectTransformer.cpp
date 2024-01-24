#include "EffectTransformer.hpp"

#include <Engine.Resource/Manage/UniqueResource.hpp>

using namespace hg::engine::accel::loader;
using namespace hg;

static void dummy_initialize_effect(Guid guid_, ref<smr<const engine::accel::AccelEffect>> effect_);

EffectTransformer::EffectTransformer() :
    Transformer() {}

EffectTransformer::response_type::type EffectTransformer::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    // TODO: Request source data to load/parse serialized acceleration effects
    //auto source = next_(next_type::next_request_type::type {}, next_type::next_request_type::options {});

    /**/

    smr<const AccelEffect> effect = nullptr;
    dummy_initialize_effect(request_->get_guid(), effect);

    /**/

    return make_smr<resource::UniqueResource<AccelEffectWrapper>>(std::move(effect));
}

/**/

#include <Engine.GFX.Render.Predefined/Effects/Skybox.hpp>

void dummy_initialize_effect(Guid guid_, ref<smr<const engine::accel::AccelEffect>> effect_) {

    if (
        auto se = engine::render::makeSkyboxEffect();
        guid_ == se->getGuid()
    ) {
        effect_ = std::move(se);
        return;
    }

}
