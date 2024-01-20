#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>

namespace hg::engine::accel {
    struct EffectCompileResultAlias {
        // Maps pipeline derivat symbols to source symbols
        Vector<_STD pair<lang::SymbolId, lang::SymbolId>> mapping;

        [[nodiscard]] constexpr lang::SymbolId aliasOrValue(cref<lang::SymbolId> value_) const noexcept {
            for (const auto& pair : mapping) {
                if (pair.first == value_) {
                    return pair.second;
                }
            }
            return value_;
        }
    };

    enum class EffectCompileResultFlag {
        eUnknown = 0x0,
        eCompiled = 0x1,
        eCached = 0x2 | eCompiled,
        eFailed = ~eUnknown
    };

    struct EffectCompileResult final {
        EffectCompileResultFlag flag;
        EffectCompileResultAlias alias;
        /**/
        smr<const AccelerationPipeline> pipeline;
    };
}
