#include "AccelerationGraphicsPass.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

AccelerationGraphicsPass::AccelerationGraphicsPass(mref<smr<AccelerationEffect>> effect_) :
    AccelerationPass(_STD move(effect_)) {}
