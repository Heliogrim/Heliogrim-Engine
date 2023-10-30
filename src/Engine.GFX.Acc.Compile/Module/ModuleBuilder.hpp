#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.GFX.Acc/Stage/StageDerivat.hpp>
#include <Engine.GFX.Acc/AccelerationPipeline.hpp>

#include "ModuleSource.hpp"
#include "../Token/ScopedTokenStorage.hpp"

namespace hg::engine::gfx::acc {
    class __declspec(novtable) ModuleBuilder {
    public:
        using this_type = ModuleBuilder;

    public:
        ModuleBuilder() noexcept = default;

        virtual ~ModuleBuilder() noexcept = default;

    protected:
        non_owning_rptr<const class Tokenizer> _tokenizer;

    public:
        [[nodiscard]] non_owning_rptr<const class Tokenizer> getTokenizer() const noexcept;

        void setTokenizer(mref<non_owning_rptr<const class Tokenizer>> tokenizer_);

    public:
        [[nodiscard]] virtual uptr<ModuleSource> build(
            cref<smr<AccelerationPipeline>> targetPass_,
            cref<class EffectSpecification> specifications_,
            cref<smr<StageDerivat>> stage_,
            cref<uptr<ModuleSource>> previous_
        ) const = 0;
    };
}
