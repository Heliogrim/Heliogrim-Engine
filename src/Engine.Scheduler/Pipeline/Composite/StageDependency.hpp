#pragma once

#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Math/Hash.hpp>

#include "../Stage/Stage.hpp"

namespace hg::engine::scheduler {
	struct StageDependency final {
		CompactSet<non_owning_rptr<const Stage>> required;

		non_owning_rptr<class StagePipeline> pipeline;
		non_owning_rptr<const Stage> stage;
	};
}

namespace std {
	template <>
	struct hash<::hg::engine::scheduler::StageDependency> :
		public std::hash<::hg::ptr<void>>,
		public std::hash<::hg::ptr<const void>> {
		[[nodiscard]] size_t operator()(
			::hg::cref<::hg::engine::scheduler::StageDependency> value_
		) const noexcept {
			size_t seed = 0;
			::hg::hash::hash_combine(
				seed,
				static_cast<::hg::cref<std::hash<::hg::ptr<void>>>>(*this)(value_.pipeline)
			);
			::hg::hash::hash_combine(
				seed,
				static_cast<::hg::cref<std::hash<::hg::ptr<const void>>>>(*this)(value_.stage)
			);
			return seed;
		}
	};

	template <>
	struct equal_to<::hg::engine::scheduler::StageDependency> {
		[[nodiscard]] bool operator()(
			::hg::cref<::hg::engine::scheduler::StageDependency> lhs_,
			::hg::cref<::hg::engine::scheduler::StageDependency> rhs_
		) const noexcept {
			return lhs_.stage == rhs_.stage && lhs_.pipeline == rhs_.pipeline;
		}
	};
}
