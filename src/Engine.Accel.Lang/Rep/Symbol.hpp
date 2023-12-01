#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Function.hpp"
#include "Variable.hpp"

namespace hg::engine::accel::lang {
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
            name(),
            var() {}

        Symbol(mref<string> name_, mref<VariableSymbol> var_) :
            name(_STD move(name_)),
            var(_STD move(var_)) {}

        Symbol(mref<string> name_, mref<FunctionSymbol> fn_) :
            name(_STD move(name_)),
            fn(_STD move(fn_)) {}

        Symbol(cref<this_type> other_) noexcept :
            name(other_.name) {
            if (other_.var.type == SymbolType::eVariableSymbol) {
                var.type = SymbolType::eVariableSymbol;
                var.data = other_.var.data;
            } else {
                fn.type = SymbolType::eFunctionSymbol;
                fn.data = other_.fn.data;
            }
        }

        Symbol(mref<this_type> other_) noexcept :
            name(_STD move(other_.name)) {
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
        string name;

        union {
            FunctionSymbol fn;
            VariableSymbol var;
        };
    };
}

namespace std {
    template <>
    struct hash<::hg::engine::accel::lang::Symbol> :
        public std::hash<string> {
        [[nodiscard]] size_t operator()(const ::hg::engine::accel::lang::Symbol& obj_) const noexcept {
            return static_cast<const std::hash<string>&>(*this)(obj_.name);
        }
    };
}
