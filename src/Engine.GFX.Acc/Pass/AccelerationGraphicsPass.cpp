#include "AccelerationGraphicsPass.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationGraphicsPass::AccelerationGraphicsPass(mref<smr<AccelerationEffect>> effect_) :
    InheritMeta(_STD move(effect_)) {}
