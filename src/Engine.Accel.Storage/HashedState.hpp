#pragma once
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Lang/Ir/Variant.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Hash.hpp>

namespace hg::engine::accel {
	class AccelerationEffect;
	class AccelerationPipeline;
	class EffectProfile;
	class EffectSpecification;
	class Stage;
	struct Permutation;
}

namespace hg::engine::accel {
	template <typename Type_>
	struct HashedState {
		size_t value;
	};

	template <typename Type_>
	inline bool operator==(cref<HashedState<Type_>> rhs_, cref<HashedState<Type_>> lhs_) noexcept {
		return rhs_.value == lhs_.value;
	}
}

namespace hg::engine::accel {
	namespace {
		template <typename Type_>
		[[nodiscard]] constexpr u64 hash_from_address(cref<Type_> obj_) noexcept {
			using pointer_type = decltype(std::addressof(obj_));
			const auto address = std::addressof(obj_);
			return ::hg::hash::fnv1a(
				std::span<char, sizeof(pointer_type)> {
					std::bit_cast<char*>(&address),
					sizeof(pointer_type)
				}
			);
		}

		[[nodiscard]] constexpr u64 hash_from_address(void* subject_) noexcept {
			return ::hg::hash::fnv1a(
				std::span<char, sizeof(subject_)> {
					std::bit_cast<char*>(&subject_), sizeof(subject_)
				}
			);
		}
	}

	constexpr HashedState<Permutation> calcStorageHash(cref<Permutation> value_) noexcept {
		constexpr auto h = std::hash<std::uintptr_t> {};
		const auto sub = std::span<const nmpt<const void>, 2uLL> {
			std::bit_cast<const ptr<const nmpt<const void>>>(std::addressof(value_)),
			2uLL
		};
		auto combined = hash_from_address(sub.front().get());
		hash::hashCombine(combined, hash_from_address(sub.back().get()));
		return { .value = combined };
	}

	constexpr HashedState<AccelerationEffect> calcStorageHash(
		cref<AccelerationEffect> value_
	) noexcept {
		return { .value = std::hash<Guid> {}(value_.getGuid()) };
	}

	constexpr HashedState<AccelerationPipeline> calcStorageHash(
		cref<AccelerationPipeline> value_
	) noexcept {
		return { .value = hash_from_address(value_) };
	}

	inline HashedState<EffectProfile> calcStorageHash(ref<const EffectProfile> value_) noexcept {
		lang::ir::ContentHasher h {};
		h.put(value_._name);

		h.put(static_cast<u64>(value_._definitions.size()));
		for (const auto& definition : value_._definitions) {
			h.put(definition.type);
			h.put(definition.data);
		}

		h.put(static_cast<u64>(value_._modules.size()));
		for (const auto& module : value_._modules) {
			h.put(module.contentHash());
		}

		/* shortcut: the profile has no toggle field yet and descriptions have no content hash, so only the usage
		 * pattern's symbol ids are keyed; add the profile toggles and a description key when they exist */
		for (const auto* const pairs : { &value_._usagePattern.ctx.pairs, &value_._usagePattern.dyn.pairs }) {
			h.put(static_cast<u64>(pairs->size()));
			for (const auto& [description, symbolId] : *pairs) {
				h.put(symbolId.value);
			}
		}

		return { .value = h.value };
	}

	constexpr HashedState<EffectSpecification> calcStorageHash(
		cref<EffectSpecification> value_
	) noexcept {
		return { .value = hash_from_address(value_) };
	}

	inline HashedState<Stage> calcStorageHash(cref<Stage> value_) noexcept {
		auto combined = static_cast<size_t>(value_.getFlagBits());
		const auto intermediate = value_.getIntermediate();
		hash::hashCombine(combined, intermediate != nullptr ? intermediate->contentHash() : 0uLL);
		return { .value = combined };
	}

	constexpr HashedState<AccelerationPipeline> combineStorageHash(
		cref<HashedState<Permutation>> hashedPermutation_,
		cref<HashedState<AccelerationEffect>> hashedEffect_
	) noexcept {

		size_t combined = 16538761uLL;
		hash::hashCombine(combined, hashedEffect_.value, hashedPermutation_.value);

		return HashedState<AccelerationPipeline> { combined };
	}
}
