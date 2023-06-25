#include "GraphicsPassSpecification.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

_STD optional<InputSlotSpec> GraphicsPassSpecification::queryInputSpec(
    cref<AccelerationStageTransferToken> token_
) const noexcept {

    const auto it = inputs.find(token_);
    if (it != inputs.end()) {
        return { it->second };
    }

    return _STD nullopt;
}

_STD optional<OutputSlotSpec> GraphicsPassSpecification::queryOutputSpec(
    cref<AccelerationStageTransferToken> token_
) const noexcept {

    const auto it = outputs.find(token_);
    if (it != outputs.end()) {
        return { it->second };
    }

    return _STD nullopt;
}
