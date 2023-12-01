#pragma once

#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>

namespace hg::engine::accel {
    enum class EffectCompileResultFlag {
        eUnknown = 0x0,
        eCompiled = 0x1,
        eCached = 0x2 | eCompiled,
        eFailed = ~eUnknown
    };

    struct EffectCompileResult final {
        EffectCompileResultFlag flag;
        smr<const AccelerationPipeline> pipeline;
    };
}
