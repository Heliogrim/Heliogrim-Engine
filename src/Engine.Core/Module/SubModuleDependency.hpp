#pragma once

#include <Engine.Common/Types.hpp>

#include "DependencyKey.hpp"

namespace hg::engine::core {
	enum class SubModuleOrder : u8 {
		eUndefined = 0x0,
		ePredecessor = 0x1,
		eSuccessor = 0x2
	};

	struct SubModuleDependency {
		DependencyKey key;
		// The order dependency of `this` module related to the dependency key
		//  -> " I'm the <Successor | Predecessor> of module X "
		SubModuleOrder order;
		bool required;
	};
}

namespace std {
	template <>
	struct hash<hg::engine::core::SubModuleDependency> :
		public hash<string_view> {
		[[nodiscard]] size_t operator()(hg::cref<hg::engine::core::SubModuleDependency> obj_) const noexcept {
			return static_cast<hg::cref<hash<string_view>>>(*this)(obj_.key);
		}
	};

	template <>
	struct equal_to<hg::engine::core::SubModuleDependency> :
		public equal_to<string_view> {
		[[nodiscard]] bool operator()(
			hg::cref<hg::engine::core::SubModuleDependency> left_,
			hg::cref<hg::engine::core::SubModuleDependency> right_
		) const noexcept {

			if (left_.required != right_.required) {
				return false;
			}

			if (left_.order != right_.order) {
				return false;
			}

			return static_cast<hg::cref<equal_to<string_view>>>(*this)(left_.key, right_.key);
		}
	};
}
