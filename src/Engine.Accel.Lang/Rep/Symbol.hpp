#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Hash/Fnv-1a.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/CompileString.hpp>

#include "Function.hpp"
#include "Variable.hpp"

namespace hg::engine::accel::lang {
	struct SymbolId final {
		using this_type = SymbolId;
		using value_type = u64;

		value_type value;

	public:
		[[nodiscard]] constexpr static this_type from(StringView stringView_) noexcept {
			constexpr auto hash = ::hg::hasher::fnv1a<StringView> {};
			return { hash(stringView_) };
		}

		[[nodiscard]] constexpr static this_type from(cref<String> string_) noexcept {
			constexpr auto hash = ::hg::hasher::fnv1a<String> {};
			return { hash(string_) };
		}

		[[nodiscard]] constexpr static this_type from(const CompileString compileString_) noexcept {
			return { compileString_.hash() };
		}

	public:
		[[nodiscard]] constexpr bool operator==(cref<this_type> other_) const noexcept {
			return value == other_.value;
		}
	};

	enum class SymbolType : bool {
		eFunctionSymbol = false,
		eVariableSymbol = true
	};

	namespace {
		struct FunctionSymbol {
		public:
			SymbolType type = SymbolType::eFunctionSymbol;
			nmpt<Function> data;

		public:
		};

		struct VariableSymbol {
		public:
			SymbolType type = SymbolType::eVariableSymbol;
			nmpt<Variable> data;
		};
	}

	struct Symbol {
	public:
		using this_type = Symbol;

	public:
		constexpr Symbol() noexcept :
			symbolId(),
			symbolName(),
			var() {}

		constexpr Symbol(mref<SymbolId> symbolId_, mref<VariableSymbol> var_) noexcept :
			symbolId(std::move(symbolId_)),
			symbolName(),
			var(std::move(var_)) {}

		constexpr Symbol(mref<SymbolId> symbolId_, mref<FunctionSymbol> fn_) noexcept:
			symbolId(std::move(symbolId_)),
			symbolName(),
			fn(std::move(fn_)) {}

		Symbol(cref<this_type> other_) noexcept :
			symbolId(other_.symbolId),
			symbolName(other_.symbolName) {
			if (other_.var.type == SymbolType::eVariableSymbol) {
				var.type = SymbolType::eVariableSymbol;
				var.data = other_.var.data;
			} else {
				fn.type = SymbolType::eFunctionSymbol;
				fn.data = other_.fn.data;
			}
		}

		Symbol(mref<this_type> other_) noexcept :
			symbolId(std::move(other_.symbolId)),
			symbolName(std::move(other_.symbolName)) {
			if (other_.var.type == SymbolType::eVariableSymbol) {
				var.type = SymbolType::eVariableSymbol;
				var.data = std::move(other_.var.data);
			} else {
				fn.type = SymbolType::eFunctionSymbol;
				fn.data = std::move(other_.fn.data);
			}
		}

		~Symbol() = default;

	public:
		SymbolId symbolId;
		String symbolName;

		union {
			FunctionSymbol fn;
			VariableSymbol var;
		};
	};
}

namespace std {
	template <>
	struct hash<::hg::engine::accel::lang::Symbol> {
		[[nodiscard]] size_t operator()(const ::hg::engine::accel::lang::Symbol& obj_) const noexcept {
			return obj_.symbolId.value;
		}
	};

	template <>
	struct hash<::hg::engine::accel::lang::SymbolId> {
		[[nodiscard]] constexpr size_t operator()(const ::hg::engine::accel::lang::SymbolId& obj_) const noexcept {
			return obj_.value;
		}
	};
}
