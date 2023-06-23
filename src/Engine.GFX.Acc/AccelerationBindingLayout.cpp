#include "AccelerationBindingLayout.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

void AccelerationBindingLayout::addDescription(cref<BindingLayoutDescription> description_) {

    for (const auto& entry : _descriptions) {
        if (entry.token == description_.token) {
            return;
        }
    }

    _descriptions.push_back(description_);
}

cref<Vector<BindingLayoutDescription>> AccelerationBindingLayout::getDescriptions() const noexcept {
    return _descriptions;
}
