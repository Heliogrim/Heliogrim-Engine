#include "Intermediate.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Make.hpp>

using namespace hg::engine::accel::lang;
using namespace hg;

Intermediate::Intermediate(cref<this_type> other_) = default;

Intermediate::Intermediate(mref<this_type> other_) noexcept = default;

void Intermediate::applyMask(u8 mask_) {
	todo_panic();
}

void Intermediate::enumerateOutboundSymbols(ref<Vector<nmpt<const Symbol>>> outbound_, u8 mask_) const noexcept {

	for (const auto& outVar : rep.globalScope.outbound) {

		auto symbol = std::ranges::find_if(
			rep.symbolTable,
			[&outVar](const auto& entry_) {

				// Warning: Check for pointer equality...
				return entry_->var.type == SymbolType::eVariableSymbol && entry_->var.data.get() == outVar.get();

				// TODO: Check masking...
			}
		);

		if (symbol != rep.symbolTable.end()) {
			outbound_.emplace_back(symbol->get());
			continue;
		}

		assert(outVar->annotation != nullptr);

		auto cur = outVar->annotation.get();
		while (cur != nullptr) {
			if (cur->type == AnnotationType::eSymbolId) {
				break;
			}
			cur = cur->next.get();
		}

		assert(cur != nullptr);

		auto next = make_uptr<Symbol>(
			SymbolId::from(static_cast<ptr<SymbolIdAnnotation>>(cur)->symbolId),
			VariableSymbol {
				.type = SymbolType::eVariableSymbol,
				.data = outVar.get()
			}
		);

		outbound_.emplace_back(next.get());
		const_cast<ref<DenseSet<uptr<Symbol>>>>(rep.symbolTable).emplace(std::move(next));
	}
}

void Intermediate::enumerateInboundSymbols(ref<Vector<nmpt<const Symbol>>> inbound_, u8 mask_) const noexcept {

	for (const auto& inVar : rep.globalScope.inbound) {

		auto symbol = std::ranges::find_if(
			rep.symbolTable,
			[&inVar](const auto& entry_) {

				// Warning: Check for pointer equality...
				return entry_->var.type == SymbolType::eVariableSymbol && entry_->var.data.get() == inVar.get();

				// TODO: Check masking...
			}
		);

		if (symbol != rep.symbolTable.end()) {
			inbound_.emplace_back(symbol->get());
			continue;
		}

		assert(inVar->annotation != nullptr);

		auto cur = inVar->annotation.get();
		while (cur != nullptr) {
			if (cur->type == AnnotationType::eSymbolId) {
				break;
			}
			cur = cur->next.get();
		}

		assert(cur != nullptr);

		auto next = make_uptr<Symbol>(
			SymbolId::from(static_cast<ptr<SymbolIdAnnotation>>(cur)->symbolId),
			VariableSymbol {
				.type = SymbolType::eVariableSymbol,
				.data = inVar.get()
			}
		);

		inbound_.emplace_back(next.get());
		const_cast<ref<DenseSet<uptr<Symbol>>>>(rep.symbolTable).emplace(std::move(next));
	}
}
