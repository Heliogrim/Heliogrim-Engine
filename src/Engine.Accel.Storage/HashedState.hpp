#pragma once
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
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

	constexpr HashedState<EffectProfile> calcStorageHash(cref<EffectProfile> value_) noexcept {
		return { .value = hasher::fnv1a<decltype(value_._name)> {}(value_._name) };
	}

	constexpr HashedState<EffectSpecification> calcStorageHash(
		cref<EffectSpecification> value_
	) noexcept {
		return { .value = hash_from_address(value_) };
	}

	inline HashedState<Stage> calcStorageHash(cref<Stage> value_) noexcept {
		auto combined = static_cast<size_t>(value_.getFlagBits());
		hash::hashCombine(
			combined,
			hash_from_address(*value_.getIntermediate())
		);
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
