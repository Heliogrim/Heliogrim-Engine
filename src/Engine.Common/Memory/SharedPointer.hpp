#pragma once

#include <Engine.Pedantic/Into/IntoImpl.hpp>

#include "../__macro.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg {
	template <typename Ty_>
	using SharedPtr = std::shared_ptr<Ty_>;

	template <typename Ty_>
	using WeakPtr = std::weak_ptr<Ty_>;
}

/**/

namespace hg {
	template <typename To_, typename From_>
	struct into_impl<std::shared_ptr<To_>, std::shared_ptr<From_>> {
		[[nodiscard]] constexpr std::shared_ptr<To_> operator()(std::shared_ptr<From_>&& obj_) noexcept {
			return std::shared_ptr<To_> { std::move(obj_) };
		}
	};

	/**/

	template <typename To_, typename From_>
	struct into_impl<std::shared_ptr<To_>, std::unique_ptr<From_>> {
		[[nodiscard]] constexpr std::shared_ptr<To_> operator()(std::unique_ptr<From_>&& obj_) noexcept {
			return std::shared_ptr<To_> { std::move(obj_) };
		}
	};
}
