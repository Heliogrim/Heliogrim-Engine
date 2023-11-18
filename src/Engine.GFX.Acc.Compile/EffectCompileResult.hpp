#pragma once
#include <Engine.GFX.Acc/AccelerationPipeline.hpp>

#include "EffectPassRemapping.hpp"

namespace hg::engine::gfx::acc {
    enum class EffectCompileResultFlag {
        eUnknown = 0x0,
        eCompiled = 0x1,
        eCached = 0x2 | eCompiled,
        eFailed = ~eUnknown
    };

    struct EffectCompileResult final {
        EffectCompileResultFlag flag;
        smr<const AccelerationPipeline> pipeline;
        EffectPassRemapping remapping;
    };
}
