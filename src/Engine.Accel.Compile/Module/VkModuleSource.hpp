#pragma once
#include <ranges>
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "ModuleSource.hpp"
#include "VkBindingLocation.hpp"

namespace hg::engine::accel {
    struct MappedBindings {
    public:
        Vector<_STD pair<nmpt<const lang::Symbol>, VkBindingLocation>> inbound;
        Vector<_STD pair<nmpt<const lang::Symbol>, VkBindingLocation>> outbound;

    public:
        [[nodiscard]] cref<VkBindingLocation> inboundLocation(cref<nmpt<const lang::Symbol>> inbound_) const noexcept {
            #ifdef _DEBUG
            const auto iter = _STD ranges::find_if(
                inbound,
                [&inbound_](const auto& pair_) {
                    return inbound_ == pair_.first || inbound_->name == pair_.first->name;
                }
            );
            assert(iter != inbound.end());
            return iter->second;
            #else
            return _STD ranges::find_if(
                inbound,
                [&inbound_](const auto& pair_) {
                    return inbound_ == pair_.first || inbound_->name == pair_.first->name;
                }
            )->second;
            #endif
        }

        [[nodiscard]] nmpt<const lang::Symbol> matchInbound(cref<nmpt<const lang::Symbol>> outbound_) const noexcept {

            for (const auto& pair : inbound) {

                assert(pair.first != nullptr);
                const auto& symbol = pair.first;

                if (symbol->name == outbound_->name) {
                    assert(symbol->var.data->type == outbound_->var.data->type);
                    return symbol;
                }

            }

            return nullptr;

        }

        [[nodiscard]] cref<VkBindingLocation> outboundLocation(
            cref<nmpt<const lang::Symbol>> outbound_
        ) const noexcept {
            #ifdef _DEBUG
            const auto iter = _STD ranges::find_if(
                outbound,
                [&outbound_](const auto& pair_) {
                    return outbound_ == pair_.first || outbound_->name == pair_.first->name;
                }
            );
            assert(iter != outbound.end());
            return iter->second;
            #else
            return _STD ranges::find_if(
                outbound,
                [&outbound_](const auto& pair_) {
                    return outbound_ == pair_.first || outbound_->name == pair_.first->name;
                }
            )->second;
            #endif
        }

        [[nodiscard]] nmpt<const lang::Symbol> matchOutbound(cref<nmpt<const lang::Symbol>> inbound_) const noexcept {

            for (const auto& pair : outbound) {

                assert(pair.first != nullptr);
                const auto& symbol = pair.first;

                if (symbol->name == inbound_->name) {
                    assert(symbol->var.data->type == inbound_->var.data->type);
                    return symbol;
                }

            }

            return nullptr;
        }
    };

    struct VkModuleSource :
        public ModuleSource {
    public:
        using this_type = VkModuleSource;
        MappedBindings bindings;
    };
}
