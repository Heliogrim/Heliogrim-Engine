#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.GFX.Acc/AccelerationStageDerivat.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>

#include "ModuleSource.hpp"
#include "../Token/ScopedTokenStorage.hpp"

namespace hg::engine::gfx::acc {
    class __declspec(novtable) ModuleBuilder {
    public:
        using this_type = ModuleBuilder;

    public:
        ModuleBuilder() noexcept = default;

        virtual ~ModuleBuilder() noexcept = default;

    private:
        non_owning_rptr<const class Tokenizer> _tokenizer;

    public:
        [[nodiscard]] non_owning_rptr<const class Tokenizer> getTokenizer() const noexcept;

        void setTokenizer(mref<non_owning_rptr<const class Tokenizer>> tokenizer_);

    public:
        [[nodiscard]] virtual uptr<ModuleSource> build(
            cref<smr<AccelerationPass>> targetPass_,
            cref<class SpecificationStorage> specifications_,
            cref<smr<AccelerationStageDerivat>> stage_
        ) const = 0;
    };
}
