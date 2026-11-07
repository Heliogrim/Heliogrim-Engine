#include "Variant.hpp"

#include <algorithm>
#include <mutex>

using namespace hg::engine::accel::lang::ir;
using namespace hg;

namespace {
	/* same limits as the parser (types) and the emitter (statements, expressions); deeper means cyclic or corrupt */
	constexpr u32 type_depth_limit = 256uL;
	constexpr u32 code_depth_limit = 1024uL;

	/* stands in for an absent, out-of-range or too deep ref */
	constexpr u8 absent_marker = 0xFFu;

	struct ModuleHasher {
		ref<const Module> module;
		ContentHasher h {};

		void condition(ref<const Condition> condition_) noexcept {
			h.put(condition_.kind);
			h.put(condition_.toggle);
			h.put(condition_.value);
			h.put(static_cast<u64>(condition_.children.size()));
			for (const auto& child : condition_.children) {
				condition(child);
			}
		}

		void type(const TypeRef ref_, const u32 depth_ = 0uL) noexcept {
			if (ref_.index >= module.types.size() || depth_ > type_depth_limit) {
				h.put(absent_marker);
				return;
			}
			const auto& desc = module.types[ref_.index];
			h.put(desc.kind);
			h.put(desc.scalar);
			h.put(desc.bits);
			h.put(desc.n);
			h.put(desc.m);
			h.put(desc.count);
			h.put(desc.dim);
			h.put(desc.name);
			type(desc.element, depth_ + 1uL);
			h.put(static_cast<u64>(desc.fields.size()));
			for (const auto& field : desc.fields) {
				h.put(field.name);
				type(field.type, depth_ + 1uL);
			}
		}

		void expr(const ExprRef ref_, const u32 depth_) noexcept {
			if (ref_.index >= module.exprs.size() || depth_ > code_depth_limit) {
				h.put(absent_marker);
				return;
			}
			const auto& node = module.exprs[ref_.index];
			h.put(node.kind);
			h.put(node.name);
			/* binding and function indices are content: both declaration lists are hashed in order */
			h.put(node.binding.index);
			h.put(node.function.index);
			h.put(node.constant.index);
			h.put(node.local);
			if (node.kind == ExprKind::eConstruct) {
				type(node.type);
			}
			h.put(static_cast<u64>(node.operands.size()));
			for (const auto operand : node.operands) {
				expr(operand, depth_ + 1uL);
			}
		}

		void stmt(const StmtRef ref_, const u32 depth_) noexcept {
			if (ref_.index >= module.stmts.size() || depth_ > code_depth_limit) {
				h.put(absent_marker);
				return;
			}
			const auto& node = module.stmts[ref_.index];
			h.put(node.kind);
			h.put(node.name);
			h.put(node.mutable_);
			type(node.type);
			expr(node.lhs, depth_ + 1uL);
			expr(node.rhs, depth_ + 1uL);
			condition(node.when);
			h.put(static_cast<u64>(node.children.size()));
			for (const auto child : node.children) {
				stmt(child, depth_ + 1uL);
			}
		}

		void operator()() noexcept {
			h.put(module.name);
			h.put(module.entry);
			h.put(module.attributes.localSize[0]);
			h.put(module.attributes.localSize[1]);
			h.put(module.attributes.localSize[2]);
			h.put(module.attributes.earlyFragmentTests);

			h.put(static_cast<u64>(module.toggles.size()));
			for (const auto& toggle : module.toggles) {
				h.put(toggle.name);
				h.put(toggle.defaultValue);
				h.put(static_cast<u64>(toggle.values.size()));
				for (const auto& value : toggle.values) {
					h.put(value);
				}
			}

			h.put(static_cast<u64>(module.bindings.size()));
			for (const auto& binding : module.bindings) {
				h.put(binding.id);
				type(binding.type);
				h.put(binding.mode);
				h.put(binding.bindClass);
				h.put(binding.implicit);
				h.put(binding.input);
				condition(binding.when);
				h.put(binding.localName);
				h.put(binding.access);
				h.put(binding.packing);
			}

			h.put(static_cast<u64>(module.constants.size()));
			for (const auto& constant : module.constants) {
				h.put(constant.name);
				type(constant.type);
				expr(constant.value, 0uL);
				h.put(constant.specId.has_value());
				h.put(constant.specId.value_or(String {}));
				condition(constant.when);
			}

			h.put(static_cast<u64>(module.functions.size()));
			for (const auto& function : module.functions) {
				h.put(function.name);
				h.put(static_cast<u64>(function.params.size()));
				for (const auto& param : function.params) {
					h.put(param.name);
					h.put(param.kind);
					type(param.type);
				}
				type(function.ret);
				h.put(function.redefineAllowed);
				condition(function.when);
				h.put(function.body.has_value());
				if (function.body.has_value()) {
					stmt(function.body->root, 0uL);
				}
			}
		}
	};

	[[nodiscard]] u64 hashSorted(Vector<String> values_) {
		std::ranges::sort(values_);
		ContentHasher h {};
		h.put(static_cast<u64>(values_.size()));
		for (const auto& value : values_) {
			h.put(value);
		}
		return h.value;
	}
}

u64 hg::engine::accel::lang::ir::contentHash(ref<const Module> module_) noexcept {
	auto hasher = ModuleHasher { module_ };
	hasher();
	return hasher.h.value;
}

VariantKey hg::engine::accel::lang::ir::makeVariantKey(
	ref<const Module> module_,
	ref<const ToggleSet> toggles_,
	std::span<const String> requiredOutputs_
) {
	auto toggles = toggles_;
	std::ranges::sort(toggles);
	ContentHasher h {};
	h.put(static_cast<u64>(toggles.size()));
	for (const auto& [name, value] : toggles) {
		h.put(name);
		h.put(value);
	}

	return VariantKey {
		.module = contentHash(module_),
		.toggles = h.value,
		.outputs = hashSorted(Vector<String> { requiredOutputs_.begin(), requiredOutputs_.end() })
	};
}

smr<const Resolved> VariantCache::getOrCreate(ref<const VariantKey> key_, FnRef<smr<const Resolved>()> build_) {
	if (auto hit = find(key_); hit != nullptr) {
		return hit;
	}

	auto built = build_();
	if (built == nullptr) {
		return built;
	}

	const std::unique_lock<std::shared_mutex> lock { _mtx };
	return _entries.try_emplace(key_, std::move(built)).first->second;
}

smr<const Resolved> VariantCache::find(ref<const VariantKey> key_) const noexcept {
	const std::shared_lock<std::shared_mutex> lock { _mtx };
	const auto iter = _entries.find(key_);
	return iter != _entries.end() ? iter->second : smr<const Resolved> {};
}

smr<const ResolvedPipeline> PipelineLayoutCache::getOrCreate(
	ref<const Vector<VariantKey>> key_,
	FnRef<smr<const ResolvedPipeline>()> build_
) {
	if (auto hit = find(key_); hit != nullptr) {
		return hit;
	}

	auto built = build_();
	if (built == nullptr) {
		return built;
	}

	const std::unique_lock<std::shared_mutex> lock { _mtx };
	return _entries.try_emplace(key_, std::move(built)).first->second;
}

smr<const ResolvedPipeline> PipelineLayoutCache::find(ref<const Vector<VariantKey>> key_) const noexcept {
	const std::shared_lock<std::shared_mutex> lock { _mtx };
	const auto iter = _entries.find(key_);
	return iter != _entries.end() ? iter->second : smr<const ResolvedPipeline> {};
}
