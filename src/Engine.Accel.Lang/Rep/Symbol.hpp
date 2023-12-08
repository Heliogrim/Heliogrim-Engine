#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/Hash/Fnv-1a.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Function.hpp"
#include "Variable.hpp"

namespace hg::engine::accel::lang {
    using SymbolId = AssocKey<String, hasher::fnv1a<String>>;

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
            var() {}

        Symbol(mref<SymbolId> symbolId_, mref<VariableSymbol> var_) :
            symbolId(_STD move(symbolId_)),
            var(_STD move(var_)) {}

        Symbol(mref<SymbolId> symbolId_, mref<FunctionSymbol> fn_) :
            symbolId(_STD move(symbolId_)),
            fn(_STD move(fn_)) {}

        Symbol(cref<this_type> other_) noexcept :
            symbolId(other_.symbolId) {
            if (other_.var.type == SymbolType::eVariableSymbol) {
                var.type = SymbolType::eVariableSymbol;
                var.data = other_.var.data;
            } else {
                fn.type = SymbolType::eFunctionSymbol;
                fn.data = other_.fn.data;
            }
        }

        Symbol(mref<this_type> other_) noexcept :
            symbolId(_STD move(other_.symbolId)) {
            if (other_.var.type == SymbolType::eVariableSymbol) {
                var.type = SymbolType::eVariableSymbol;
                var.data = _STD move(other_.var.data);
            } else {
                fn.type = SymbolType::eFunctionSymbol;
                fn.data = _STD move(other_.fn.data);
            }
        }

        ~Symbol() = default;

    public:
        SymbolId symbolId;

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
            return obj_.symbolId.hash;
        }
    };
}
