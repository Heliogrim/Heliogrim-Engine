#include "Intermediate.hpp"

#include <Engine.Common/Make.hpp>

using namespace hg::engine::accel::lang;
using namespace hg;

Intermediate::Intermediate(cref<this_type> other_) = default;

Intermediate::Intermediate(mref<this_type> other_) noexcept = default;

void Intermediate::applyMask(u8 mask_) {
    _STD unreachable();
}

void Intermediate::enumerateOutboundSymbols(ref<Vector<nmpt<const Symbol>>> outbound_, u8 mask_) const noexcept {

    for (const auto& outVar : rep.globalScope.outbound) {

        auto symbol = _STD ranges::find_if(
            rep.symbolTable,
            [&outVar](const auto& entry_) {

                // Warning: Check for pointer equality...
                return entry_->var.type == SymbolType::eFunctionSymbol && entry_->var.data.get() == outVar.get();

                // TODO: Check masking...
            }
        );

        if (symbol != rep.symbolTable.end()) {
            outbound_.emplace_back(symbol->get());
            continue;
        }

        auto next = make_uptr<Symbol>(
            "",
            VariableSymbol {
                .type = SymbolType::eVariableSymbol,
                .data = outVar.get()
            }
        );

        // TODO: Should not happen | Found outbound which is not in symbol table
        __debugbreak();

        outbound_.emplace_back(next.get());
        const_cast<ref<DenseSet<uptr<Symbol>>>>(rep.symbolTable).emplace(_STD move(next));
    }
}

void Intermediate::enumerateInboundSymbols(ref<Vector<nmpt<const Symbol>>> inbound_, u8 mask_) const noexcept {

    for (const auto& inVar : rep.globalScope.inbound) {

        auto symbol = _STD ranges::find_if(
            rep.symbolTable,
            [&inVar](const auto& entry_) {

                // Warning: Check for pointer equality...
                return entry_->var.type == SymbolType::eFunctionSymbol && entry_->var.data.get() == inVar.get();

                // TODO: Check masking...
            }
        );

        if (symbol != rep.symbolTable.end()) {
            inbound_.emplace_back(symbol->get());
            continue;
        }

        auto next = make_uptr<Symbol>(
            "",
            VariableSymbol {
                .type = SymbolType::eVariableSymbol,
                .data = inVar.get()
            }
        );

        // TODO: Should not happen | Found inbound which is not in symbol table
        __debugbreak();

        inbound_.emplace_back(next.get());
        const_cast<ref<DenseSet<uptr<Symbol>>>>(rep.symbolTable).emplace(_STD move(next));
    }
}
