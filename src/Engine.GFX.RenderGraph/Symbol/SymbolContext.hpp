#pragma once
#include <map>
#include <variant>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>

#include "Observed.hpp"
#include "Subscribed.hpp"
#include "SymbolizedResource.hpp"

namespace hg::engine::gfx::render::graph {
    class SymbolContext {
    public:
        using this_type = SymbolContext;

        using observe_type = Observed<SymbolizedResource>;
        using subscribe_type = Subscribed<SymbolizedResource>;

    public:
        SymbolContext() noexcept;

        SymbolContext(mref<this_type>) = delete;

        SymbolContext(cref<this_type>) = delete;

        ~SymbolContext() noexcept;

    private:
        void tidy();

    private:
        nmpt<this_type> _parent;

    public:
        [[nodiscard]] bool isRoot() const noexcept;

        [[nodiscard]] nmpt<const this_type> root() const noexcept;

        [[nodiscard]] nmpt<this_type> parent() const noexcept;

    private:
        std::map<smr<const acc::Symbol>, nmpt<SymbolizedResource>> _resources;
        Vector<
            _STD pair<
                smr<const acc::Symbol>,
                Vector<_STD variant<const ptr<observe_type>, const ptr<subscribe_type>>>
            >
        > _register;

    public:
        void exposeSymbol(mref<smr<const acc::Symbol>> symbol_, nmpt<SymbolizedResource> resource_);

        [[nodiscard]] nmpt<SymbolizedResource> exportSymbol(mref<smr<const acc::Symbol>> symbol_);

        [[nodiscard]] nmpt<SymbolizedResource> importSymbol(mref<smr<const acc::Symbol>> symbol_);

    public:
        bool registerExposeSymbol(
            mref<smr<const acc::Symbol>> symbol_,
            const ptr<Observed<SymbolizedResource>> observation_
        );

        bool registerExportSymbol(
            mref<smr<const acc::Symbol>> symbol_,
            const ptr<Subscribed<SymbolizedResource>> subscription_
        );

        bool registerImportSymbol(
            mref<smr<const acc::Symbol>> symbol_,
            const ptr<Subscribed<SymbolizedResource>> subscription_
        );

    public:
        [[nodiscard]] _Success_(return != nullptr) nmpt<SymbolizedResource> getExportSymbol(
            mref<smr<const acc::Symbol>> symbol_
        ) const;

        [[nodiscard]] _Success_(return != nullptr) nmpt<SymbolizedResource> getImportSymbol(
            mref<smr<const acc::Symbol>> symbol_
        ) const;

    public:
        bool eraseExposedSymbol(mref<smr<const acc::Symbol>> symbol_);

        bool eraseExposedSymbol(mref<nmpt<SymbolizedResource>> resource_);

        [[nodiscard]] bool eraseExportSymbol(mref<smr<const acc::Symbol>> symbol_);

        [[nodiscard]] bool eraseExportSymbol(mref<nmpt<SymbolizedResource>> resource_);

        [[nodiscard]] bool eraseImportSymbol(mref<smr<const acc::Symbol>> symbol_);

        [[nodiscard]] bool eraseImportSymbol(mref<nmpt<SymbolizedResource>> resource_);

    public:
        [[nodiscard]] bool isShadowing(mref<smr<const acc::Symbol>> symbol_);
    };
}
