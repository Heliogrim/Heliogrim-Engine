#pragma once
#include <algorithm>
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
		Vector<std::pair<nmpt<const lang::Symbol>, VkBindingLocation>> inbound;
		Vector<std::pair<nmpt<const lang::Symbol>, VkBindingLocation>> outbound;

	public:
		[[nodiscard]] cref<VkBindingLocation> inboundLocation(cref<nmpt<const lang::Symbol>> inbound_) const noexcept {
			#ifdef _DEBUG
			const auto iter = std::ranges::find_if(
				inbound,
				[&inbound_](const auto& pair_) {
					return inbound_ == pair_.first || inbound_->symbolId == pair_.first->symbolId;
				}
			);
			assert(iter != inbound.end());
			return iter->second;
			#else
			return std::ranges::find_if(
				inbound,
				[&inbound_](const auto& pair_) {
					return inbound_ == pair_.first || inbound_->symbolId == pair_.first->symbolId;
				}
			)->second;
			#endif
		}

		[[nodiscard]] nmpt<const lang::Symbol> matchInbound(cref<nmpt<const lang::Symbol>> outbound_) const noexcept {

			for (const auto& pair : inbound) {

				assert(pair.first != nullptr);
				const auto& symbol = pair.first;

				if (symbol->symbolId == outbound_->symbolId) {
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
			const auto iter = std::ranges::find_if(
				outbound,
				[&outbound_](const auto& pair_) {
					return outbound_ == pair_.first || outbound_->symbolId == pair_.first->symbolId;
				}
			);
			assert(iter != outbound.end());
			return iter->second;
			#else
			return std::ranges::find_if(
				outbound,
				[&outbound_](const auto& pair_) {
					return outbound_ == pair_.first || outbound_->symbolId == pair_.first->symbolId;
				}
			)->second;
			#endif
		}

		[[nodiscard]] nmpt<const lang::Symbol> matchOutbound(cref<nmpt<const lang::Symbol>> inbound_) const noexcept {

			for (const auto& pair : outbound) {

				assert(pair.first != nullptr);
				const auto& symbol = pair.first;

				if (symbol->symbolId == inbound_->symbolId) {
					assert(symbol->var.data->type == inbound_->var.data->type);
					return symbol;
				}

			}

			return nullptr;
		}
	};

	struct VkModuleSource : public ModuleSource {
		constexpr ~VkModuleSource() noexcept override = default;

		using this_type = VkModuleSource;
		MappedBindings bindings;
	};
}
