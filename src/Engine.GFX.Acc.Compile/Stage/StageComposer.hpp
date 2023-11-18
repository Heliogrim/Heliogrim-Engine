#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX.Acc/AccelerationPipeline.hpp>

namespace hg::engine::gfx::acc {
    class StageComposer {
    public:
        using this_type = StageComposer;

    public:
        StageComposer() noexcept = default;

        ~StageComposer() noexcept = default;

    private:
        non_owning_rptr<const class Tokenizer> _tokenizer;

    public:
        [[nodiscard]] non_owning_rptr<const class Tokenizer> getTokenizer() const noexcept;

        void setTokenizer(mref<non_owning_rptr<const class Tokenizer>> tokenizer_);

    public:
        [[nodiscard]] Vector<smr<StageDerivat>> compose(
            cref<smr<const class EffectProfile>> profile_,
            cref<smr<const class EffectSpecification>> specifications_,
            cref<smr<AccelerationPipeline>> targetPass_
        ) const;
    };
}
