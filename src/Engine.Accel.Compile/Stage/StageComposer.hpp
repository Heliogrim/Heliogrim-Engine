#pragma once

#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Accel.Pipeline/Stage/StageDerivat.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

namespace hg::engine::accel {
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
