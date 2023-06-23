#include "AccelerationInputLayout.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

void AccelerationInputLayout::addDescription(cref<InputLayoutDescription> description_) {

    for (const auto& entry : _descriptions) {
        if (entry.token == description_.token) {
            return;
        }
    }

    _descriptions.push_back(description_);
}

cref<Vector<InputLayoutDescription>> AccelerationInputLayout::getDescriptions() const noexcept {
    return _descriptions;
}
