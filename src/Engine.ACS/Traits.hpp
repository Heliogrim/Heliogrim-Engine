#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Hash/Murmur3.hpp>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Heliogrim/Actor/ActorGuid.hpp>

#include "ComponentGuid.hpp"

namespace hg {
	/**
	 * Identifier for the invalid actor
	 */
	inline constexpr static ActorGuid invalid_actor_guid {};

	/**
	 * Identifier for the invalid component
	 */
	inline constexpr static ComponentGuid invalid_component_guid {};

	/**
	 * Generates an actor unique identifier
	 *
	 * @author Julius
	 * @date 23.10.2020
	 *
	 * @returns The actor unique identifier.
	 */
	[[nodiscard]] ActorGuid generate_actor_guid();

	/**
	 * Generates an component unique identifier
	 * @returns The component unique identifier.
	 */
	[[nodiscard]] ComponentGuid generate_component_guid();
}

namespace std {
	/**
	 * Specific hash of actor_guid
	 *
	 * @author Julius
	 * @date 29.10.2020
	 */
	template <>
	struct hash<hg::ActorGuid> {
		std::size_t operator()(const hg::ActorGuid& value_) const noexcept {
			/**
			 * !important! don't use identity
			 */
			hg::u64 dst[2];
			hg::hash::murmur3_x64_128(
				&value_,
				sizeof(hg::ActorGuid),
				0x9FB21C651E98DF25uLL,
				&dst
			);
			return dst[0];
		}
	};

	/**
	 * Specific hash of ComponentGuid
	 *
	 * @author Julius
	 * @date 08.12.2025
	 */
	template <>
	struct hash<hg::ComponentGuid> {
		std::size_t operator()(const hg::ComponentGuid& value_) const noexcept {
			/**
			 * !important! don't use identity
			 */
			hg::u64 dst[2];
			hg::hash::murmur3_x64_128(
				&value_,
				sizeof(hg::ComponentGuid),
				0x9FB21C651E98DF25uLL,
				&dst
			);
			return dst[0];
		}
	};
}
