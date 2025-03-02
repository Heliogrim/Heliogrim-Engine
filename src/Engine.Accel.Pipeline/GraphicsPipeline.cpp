#include "GraphicsPipeline.hpp"

using namespace hg::engine::accel;
using namespace hg;

GraphicsPipeline::GraphicsPipeline(mref<smr<const AccelerationEffect>> effect_) :
	InheritMeta(std::move(effect_)) {}
