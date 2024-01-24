#pragma once
#include <map>
#include <variant>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Observed.hpp"
#include "Subscribed.hpp"
#include "SymbolizedResource.hpp"
#include "Symbol.hpp"

namespace hg::engine::render::graph {
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
        std::map<
            smr<const Symbol>, nmpt<SymbolizedResource>,
            decltype([](cref<smr<const Symbol>> left_, cref<smr<const Symbol>> right_) -> bool {
                return left_.get() < right_.get();
            })
        > _resources;
        Vector<
            std::pair<
                smr<const Symbol>,
                Vector<std::variant<const ptr<observe_type>, const ptr<subscribe_type>>>
            >
        > _register;

    public:
        void exposeSymbol(mref<smr<const Symbol>> symbol_, nmpt<SymbolizedResource> resource_);

        [[nodiscard]] nmpt<SymbolizedResource> exportSymbol(mref<smr<const Symbol>> symbol_);

        [[nodiscard]] nmpt<SymbolizedResource> importSymbol(mref<smr<const Symbol>> symbol_);

    public:
        bool registerExposeSymbol(
            mref<smr<const Symbol>> symbol_,
            const ptr<Observed<SymbolizedResource>> observation_
        );

        bool registerExportSymbol(
            mref<smr<const Symbol>> symbol_,
            const ptr<Subscribed<SymbolizedResource>> subscription_
        );

        bool registerImportSymbol(
            mref<smr<const Symbol>> symbol_,
            const ptr<Subscribed<SymbolizedResource>> subscription_
        );

    public:
        [[nodiscard]] _Success_(return != nullptr) nmpt<SymbolizedResource> getExportSymbol(
            mref<smr<const Symbol>> symbol_
        ) const;

        [[nodiscard]] _Success_(return != nullptr) nmpt<SymbolizedResource> getImportSymbol(
            mref<smr<const Symbol>> symbol_
        ) const;

    public:
        bool eraseExposedSymbol(mref<smr<const Symbol>> symbol_);

        bool eraseExposedSymbol(mref<nmpt<SymbolizedResource>> resource_);

        [[nodiscard]] bool eraseExportSymbol(mref<smr<const Symbol>> symbol_);

        [[nodiscard]] bool eraseExportSymbol(mref<nmpt<SymbolizedResource>> resource_);

        [[nodiscard]] bool eraseImportSymbol(mref<smr<const Symbol>> symbol_);

        [[nodiscard]] bool eraseImportSymbol(mref<nmpt<SymbolizedResource>> resource_);

    public:
        [[nodiscard]] bool isShadowing(mref<smr<const Symbol>> symbol_);
    };
}
