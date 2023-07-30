#include "GraphicsPass.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

GraphicsPass::GraphicsPass(mref<smr<AccelerationEffect>> effect_) :
    InheritMeta(_STD move(effect_)) {}
