#pragma once

#include <Engine.Async/Primitive/FiberSharedMutex.hpp>
#include <Engine.Common/__macro.hpp>

#include "../Intermediate.hpp"
#include "Condition.hpp"
#include "Interface.hpp"
#include "Layout.hpp"
#include "Module.hpp"
#include "Tier.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
	[[nodiscard]] u64 contentHash(ref<const Module> mod_) noexcept;

	struct VariantKey {
		u64 mod = 0uLL;
		u64 toggles = 0uLL;
		u64 outputs = 0uLL;

		[[nodiscard]] bool operator==(const VariantKey&) const noexcept = default;
	};

	/**
	 * Toggles and output ids are sorted before hashing, so their order does not matter.
	 */
	[[nodiscard]] VariantKey makeVariantKey(
		ref<const Module> mod_,
		ref<const ToggleSet> toggles_,
		std::span<const String> requiredOutputs_
	);

	/**
	 * One resolved variant: the resolved and sliced module and its interface. Layout is per pipeline (see
	 * `ResolvedPipeline`), as it depends on the other stages.
	 */
	struct Resolved {
		Module mod;
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
		size_t seed = key_.mod;
		::hg::hash::hashCombine(seed, key_.toggles, key_.outputs);
		return seed;
	}
};

namespace hg::engine::accel::lang::ir {
	/**
	 * Thread-safe map from variant key to resolved variant; a hit is one hash lookup under a fiber-safe spin lock.
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
		mutable concurrent::FiberSharedMutex _lck;
		DenseMap<VariantKey, smr<const Resolved>, std::hash<VariantKey>> _entries;
	};

	/**
	 * A pipeline's stage keys in order, plus what the layout and lowering depend on beyond them: the follow mask
	 * (bit k set when stage k follows its predecessor's interface) and the emission tier.
	 */
	struct PipelineKey {
		Vector<VariantKey> stages;
		u64 follows = 0uLL;
		Tier tier = Tier::eBuild;

		[[nodiscard]] bool operator==(const PipelineKey&) const noexcept = default;
	};

	struct PipelineKeyHash {
		[[nodiscard]] size_t operator()(ref<const PipelineKey> key_) const noexcept {
			size_t seed = key_.stages.size();
			for (const auto& stage : key_.stages) {
				::hg::hash::hashCombine(seed, std::hash<VariantKey> {}(stage));
			}
			::hg::hash::hashCombine(seed, key_.follows);
			::hg::hash::hashCombine(seed, static_cast<u64>(key_.tier));
			return seed;
		}
	};

	/**
	 * Thread-safe map from a pipeline key to its resolved pipeline; same contract
	 * as `VariantCache`: a hit is one hash lookup under the lock, a null build result is not cached.
	 */
	class PipelineLayoutCache {
	public:
		[[nodiscard]] smr<const ResolvedPipeline> getOrCreate(
			ref<const PipelineKey> key_,
			FnRef<smr<const ResolvedPipeline>()> build_
		);

		[[nodiscard]] smr<const ResolvedPipeline> find(ref<const PipelineKey> key_) const noexcept;

	private:
		mutable concurrent::FiberSharedMutex _lck;
		DenseMap<PipelineKey, smr<const ResolvedPipeline>, PipelineKeyHash> _entries;
	};
}
