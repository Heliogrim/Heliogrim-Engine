#pragma once

#include "../Component.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) SubpassAccelComponent :
        public InheritMeta<SubpassAccelComponent, Component> {
    public:
        using this_type = SubpassAccelComponent;

    public:
        SubpassAccelComponent() noexcept = default;

        ~SubpassAccelComponent() noexcept override = default;

    public:
        [[nodiscard]] virtual Vector<smr<acc::Symbol>> aggregateImportedSymbols() const = 0;

        [[nodiscard]] virtual Vector<smr<acc::Symbol>> aggregateExportedSymbols() const = 0;
    };
}
