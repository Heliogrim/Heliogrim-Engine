#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "Stage/Stage.hpp"

namespace hg::engine::accel {
    class AccelerationEffect {
    public:
        using this_type = AccelerationEffect;

    public:
        AccelerationEffect() noexcept = default;

        AccelerationEffect(
            mref<Guid> guid_,
            mref<string> name_,
            mref<Vector<smr<Stage>>> stages_
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
        Vector<smr<Stage>> _stages;

    public:
        [[nodiscard]] cref<Vector<smr<Stage>>> getStages() const noexcept;

        void enumerateImportSymbols(
            _Out_ ref<Vector<StageInput>> imports_
        ) const noexcept;

        void enumerateExportSymbols(
            _Out_ ref<Vector<StageOutput>> exports_
        ) const noexcept;

    public:
        [[nodiscard]] Optional<StageInput> getFirstInputFor(cref<lang::Symbol> symbol_) const noexcept;

        [[nodiscard]] Optional<StageOutput> getLastOutputFor(cref<lang::Symbol> symbol_) const noexcept;
    };
}
