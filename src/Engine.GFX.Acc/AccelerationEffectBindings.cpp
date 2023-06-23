#include "AccelerationEffectBindings.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

void AccelerationEffectBindings::addDescription(cref<BindingLayoutDescription> description_) {

    for (const auto& entry : _descriptions) {
        if (entry.token == description_.token) {
            return;
        }
    }

    _descriptions.push_back(description_);
}

cref<Vector<BindingLayoutDescription>> AccelerationEffectBindings::getDescriptions() const noexcept {
    return _descriptions;
}
