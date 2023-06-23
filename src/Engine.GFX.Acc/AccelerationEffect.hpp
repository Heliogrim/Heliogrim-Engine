#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "__fwd.hpp"

#include "AccelerationStage.hpp"
#include "AccelerationEffectInputs.hpp"
#include "AccelerationEffectBindings.hpp"
#include "AccelerationEffectOutputs.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationEffect {
    public:
        using this_type = AccelerationEffect;

    public:
        AccelerationEffect() noexcept = default;

        AccelerationEffect(
            mref<Guid> guid_,
            mref<string> name_,
            mref<Vector<smr<AccelerationStage>>> stages_,
            mref<AccelerationEffectInputs> inputs_,
            mref<AccelerationEffectBindings> bindings_,
            mref<AccelerationEffectOutputs> outputs_
        );

        ~AccelerationEffect();

    private:
        Guid _guid;

    public:
        [[nodiscard]] Guid getGuid() const noexcept;

    private:
        string _name;

    public:
        [[nodiscard]] string getName() const noexcept;

    private:
        Vector<smr<AccelerationStage>> _stages;

        AccelerationEffectInputs _inputs;
        AccelerationEffectBindings _bindings;
        AccelerationEffectOutputs _outputs;

    public:
        [[nodiscard]] cref<Vector<smr<AccelerationStage>>> getStages() const noexcept;

        [[nodiscard]] cref<AccelerationEffectInputs> getInputLayout() const noexcept;

        [[nodiscard]] cref<AccelerationEffectBindings> getBindingLayout() const noexcept;

        [[nodiscard]] cref<AccelerationEffectOutputs> getOutputLayout() const noexcept;
    };
}
