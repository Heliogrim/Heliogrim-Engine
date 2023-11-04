#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "__fwd.hpp"

#include "Stage/Stage.hpp"
#include "Symbol/Symbol.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationEffect {
    public:
        using this_type = AccelerationEffect;

    public:
        AccelerationEffect() noexcept = default;

        AccelerationEffect(
            mref<Guid> guid_,
            mref<string> name_,
            mref<Vector<smr<Stage>>> stages_,
            mref<Vector<smr<const Symbol>>> importSymbols_,
            mref<Vector<smr<const Symbol>>> exportSymbols_
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
        Vector<smr<const Symbol>> _importSymbols;
        Vector<smr<const Symbol>> _exportSymbols;

    public:
        [[nodiscard]] cref<Vector<smr<Stage>>> getStages() const noexcept;

        [[nodiscard]] cref<Vector<smr<const Symbol>>> getImportSymbols() const noexcept;

        [[nodiscard]] cref<Vector<smr<const Symbol>>> getExportSymbols() const noexcept;

    public:
        [[nodiscard]] Optional<ref<StageInput>> getFirstInputFor(cref<Symbol> symbol_) const noexcept;

        [[nodiscard]] Optional<ref<StageOutput>> getLastOutputFor(cref<Symbol> symbol_) const noexcept;
    };
}
