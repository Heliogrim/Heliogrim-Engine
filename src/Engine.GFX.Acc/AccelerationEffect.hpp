#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "__fwd.hpp"

#include "AccelerationStage.hpp"
#include "AccelerationInputLayout.hpp"
#include "AccelerationBindingLayout.hpp"
#include "AccelerationOutputLayout.hpp"

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
            mref<AccelerationInputLayout> inputLayout_,
            mref<AccelerationBindingLayout> bindingLayout_,
            mref<AccelerationOutputLayout> outputLayout_
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

        AccelerationInputLayout _inputLayout;
        AccelerationBindingLayout _bindingLayout;
        AccelerationOutputLayout _outputLayout;

    public:
        [[nodiscard]] cref<Vector<smr<AccelerationStage>>> getStages() const noexcept;

        [[nodiscard]] cref<AccelerationInputLayout> getInputLayout() const noexcept;

        [[nodiscard]] cref<AccelerationBindingLayout> getBindingLayout() const noexcept;

        [[nodiscard]] cref<AccelerationOutputLayout> getOutputLayout() const noexcept;
    };
}
