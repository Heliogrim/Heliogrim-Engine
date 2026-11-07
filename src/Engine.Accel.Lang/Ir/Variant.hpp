#pragma once

#include <array>
#include <bit>
#include <concepts>
#include <functional>
#include <shared_mutex>
#include <span>
#include <utility>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Functional/FnRef.hpp>
#include <Engine.Common/Math/Hash.hpp>

#include "Condition.hpp"
#include "Interface.hpp"
#include "Layout.hpp"
#include "Module.hpp"
#include "../Intermediate.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Streaming 64-bit FNV-1a; strings are length-prefixed so concatenations cannot collide.
	 */
	struct ContentHasher {
		u64 value = 14695981039346656037uLL;

		void mix(const unsigned char byte_) noexcept {
			value = (value ^ byte_) * 1099511628211uLL;
		}

		template <typename Type_> requires std::integral<Type_> || std::is_enum_v<Type_>
		void put(const Type_ value_) noexcept {
			for (const auto byte : std::bit_cast<std::array<unsigned char, sizeof(Type_)>>(value_)) {
				mix(byte);
			}
		}

		void put(const StringView value_) noexcept {
			put(static_cast<u64>(value_.size()));
			for (const auto ch : value_) {
				mix(static_cast<unsigned char>(ch));
			}
		}
	};

	/**
	 * Hash of the module's content: name, entry, toggles, declarations and the code reachable from function bodies.
	 * Types are hashed by structure where referenced (never by arena index) and unreachable arena nodes are never
	 * visited, so equal modules hash equal whatever their arenas hold besides the live code.
	 */
	[[nodiscard]] u64 contentHash(ref<const Module> module_) noexcept;

	struct VariantKey {
		u64 module = 0uLL;
		u64 toggles = 0uLL;
		u64 outputs = 0uLL;

		[[nodiscard]] bool operator==(const VariantKey&) const noexcept = default;
	};

	/**
	 * Toggles and output ids are sorted before hashing, so their order does not matter.
	 */
	[[nodiscard]] VariantKey makeVariantKey(
		ref<const Module> module_,
		ref<const ToggleSet> toggles_,
		std::span<const String> requiredOutputs_
	);

	/**
	 * One resolved variant: the resolved and sliced module and its interface. Layout is per pipeline (see
	 * `ResolvedPipeline`), as it depends on the other stages.
	 */
	struct Resolved {
		Module module;
		Vector<InterfaceEntry> interfaceEntries;
		/* the key it is cached under, so a pipeline key is built without hashing the inputs again */
		VariantKey key;
	};

	/**
	 * The stages of one pipeline laid out together: per stage, in stage order, its variant, its layout (shared ids
	 * have one slot in every stage) and its lowered legacy form (copy it for each user).
	 */
	struct ResolvedPipeline {
		Vector<smr<const Resolved>> stages;
		Vector<Layout> layouts;
		Vector<Intermediate> lowered;
	};
}

template <>
struct std::hash<::hg::engine::accel::lang::ir::VariantKey> {
	[[nodiscard]] size_t operator()(const ::hg::engine::accel::lang::ir::VariantKey& key_) const noexcept {
		size_t seed = key_.module;
		::hg::hash::hashCombine(seed, key_.toggles, key_.outputs);
		return seed;
	}
};

namespace hg::engine::accel::lang::ir {
	/**
	 * Thread-safe map from variant key to resolved variant; a hit is one hash lookup under a shared lock.
	 */
	class VariantCache {
	public:
		/**
		 * Returns the cached variant or builds it outside the lock; if two threads build the same key, the first
		 * stored result wins and is returned to both. A null build result is not cached.
		 */
		[[nodiscard]] smr<const Resolved> getOrCreate(ref<const VariantKey> key_, FnRef<smr<const Resolved>()> build_);

		[[nodiscard]] smr<const Resolved> find(ref<const VariantKey> key_) const noexcept;

	private:
		mutable std::shared_mutex _mtx;
		DenseMap<VariantKey, smr<const Resolved>, std::hash<VariantKey>> _entries;
	};

	struct PipelineKeyHash {
		[[nodiscard]] size_t operator()(const Vector<VariantKey>& key_) const noexcept {
			size_t seed = key_.size();
			for (const auto& stage : key_) {
				::hg::hash::hashCombine(seed, std::hash<VariantKey> {}(stage));
			}
			return seed;
		}
	};

	/**
	 * Thread-safe map from the ordered variant keys of a pipeline's stages to its resolved pipeline; same contract
	 * as `VariantCache`: a hit is one hash lookup under a shared lock, a null build result is not cached.
	 */
	class PipelineLayoutCache {
	public:
		[[nodiscard]] smr<const ResolvedPipeline> getOrCreate(
			ref<const Vector<VariantKey>> key_,
			FnRef<smr<const ResolvedPipeline>()> build_
		);

		[[nodiscard]] smr<const ResolvedPipeline> find(ref<const Vector<VariantKey>> key_) const noexcept;

	private:
		mutable std::shared_mutex _mtx;
		DenseMap<Vector<VariantKey>, smr<const ResolvedPipeline>, PipelineKeyHash> _entries;
	};
}
