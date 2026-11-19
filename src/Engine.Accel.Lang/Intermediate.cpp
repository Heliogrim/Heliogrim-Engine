#include "Intermediate.hpp"

#include <Engine.Common/__macro.hpp>
#include <cassert>

#include "Ir/Variant.hpp"

import Engine.Asserts;
import Engine.Common;

using namespace hg::engine::accel::lang;
using namespace hg;

Intermediate::Intermediate(cref<this_type> other_) = default;

Intermediate::Intermediate(mref<this_type> other_) noexcept = default;

u64 Intermediate::contentHash() const noexcept {
	ir::ContentHasher h {};
	h.put(lang.dialect);

	h.put(static_cast<u64>(lang.text.size()));
	for (const auto& block : lang.text) {
		h.put(block.data);
	}

	h.put(static_cast<u64>(spirv.size()));
	for (const auto word : spirv) {
		h.put(word);
	}

	/* shortcut: legacy rep functions, mainEntry and symbol table are not hashed (symbols are derived from the
	 * variables' SymbolId annotations; lowering emits no rep functions), hash them once hand-built ones carry code */
	const auto variables = [&h](ref<const Vector<uptr<Variable>>> variables_) {
		h.put(static_cast<u64>(variables_.size()));
		for (const auto& variable : variables_) {
			h.put(variable->type.category);
			h.put(variable->type.typeData);
			for (auto cur = variable->annotation.get(); cur != nullptr; cur = cur->next.get()) {
				h.put(cur->type);
				if (cur->type == AnnotationType::eSymbolId) {
					h.put(static_cast<ptr<const SymbolIdAnnotation>>(cur)->symbolId);

				} else if (cur->type == AnnotationType::eVkBindLocation) {
					const auto* const location = static_cast<ptr<const VkBindLocationAnnotation>>(cur);
					h.put(location->vkSet);
					h.put(location->vkLocation);
					h.put(location->vkOffset);
					h.put(location->vkFormat);
				}
			}
			/* end of the chain, so chains cannot shift into the next variable */
			h.put(u8 { 0xFFu });
		}
	};
	variables(rep.globalScope.inbound);
	variables(rep.globalScope.outbound);
	variables(rep.globalScope.variables);

	h.put(interface.stage);
	h.put(static_cast<u64>(interface.entries.size()));
	for (const auto& entry : interface.entries) {
		h.put(entry.id);
		h.put(entry.type.index);
		h.put(entry.mode);
		h.put(entry.cls);
		h.put(entry.implicit);
		h.put(entry.input);
		h.put(entry.arrayed);
		h.put(entry.typeName);
		h.put(entry.patch);
	}
	for (const auto size : interface.workgroupSize) {
		h.put(size);
	}
	h.put(static_cast<u64>(interface.specConstantIds.size()));
	for (const auto& id : interface.specConstantIds) {
		h.put(id);
	}

	h.put(ir != nullptr);
	if (ir != nullptr) {
		h.put(ir::contentHash(*ir));
	}
	return h.value;
}

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
