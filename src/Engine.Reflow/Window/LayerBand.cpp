#include "LayerBand.hpp"

using namespace hg::engine::reflow;
using namespace hg;

bool hg::engine::reflow::isHitTestable(const LayerBand band_) noexcept {
	return band_ != LayerBand::eDragPreview && band_ != LayerBand::eTooltip;
}
