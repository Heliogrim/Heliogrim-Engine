#include "Representation.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Common/Make.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::accel;
using namespace hg;

lang::Representation::Representation(cref<this_type> other_) :
	globalScope(),
	mainEntry(),
	symbolTable() {
	// TODO: Deep copy global scope
	// TODO: Query mainEntry in new scope
	// TODO: Rebuild symbol table

	for (const auto& outbound : other_.globalScope.outbound) {
		globalScope.outbound.emplace_back(make_uptr<Variable>(*outbound));
	}

	for (const auto& inbound : other_.globalScope.inbound) {
		globalScope.inbound.emplace_back(make_uptr<Variable>(*inbound));
	}

	assert(other_.mainEntry == nullptr);

	for (const auto& symbol : other_.symbolTable) {

		if (symbol->fn.type == SymbolType::eFunctionSymbol) {

			// TODO: Migrate back reference into representation

			symbolTable.emplace(
				make_uptr<Symbol>(
					clone(symbol->symbolId),
					FunctionSymbol {
						SymbolType::eFunctionSymbol,
						nullptr
					}
				)
			);

		} else if (symbol->fn.type == SymbolType::eVariableSymbol) {

			Vector<uptr<Variable>>* set = nullptr;

			const auto iter = std::ranges::find(
				other_.globalScope.outbound,
				symbol->var.data.get(),
				[](const auto& outbound_) {
					return outbound_.get();
				}
			);

			ptrdiff_t offset = 0;
			if (iter != other_.globalScope.outbound.end()) {
				set = &globalScope.outbound;
				offset = std::distance(other_.globalScope.outbound.begin(), iter);

			}

			if (set == nullptr) {
				const auto iter = std::ranges::find(
					other_.globalScope.inbound,
					symbol->var.data.get(),
					[](const auto& inbound_) {
						return inbound_.get();
					}
				);

				if (iter != other_.globalScope.inbound.end()) {
					set = &globalScope.inbound;
					offset = std::distance(other_.globalScope.inbound.begin(), iter);
				}
			}

			assert(set != nullptr);

			const auto& bref = set->at(offset);
			symbolTable.emplace(
				make_uptr<Symbol>(
					clone(symbol->symbolId),
					VariableSymbol {
						SymbolType::eVariableSymbol,
						bref.get()
					}
				)
			);
		}
	}
}
