#include "AccelerationOutputLayout.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

void AccelerationOutputLayout::addDescription(cref<OutputLayoutDescription> description_) {

    for (const auto& entry : _descriptions) {
        if (entry.token == description_.token) {
            return;
        }
    }

    _descriptions.push_back(description_);
}

cref<Vector<OutputLayoutDescription>> AccelerationOutputLayout::getDescriptions() const noexcept {
    return _descriptions;
}
