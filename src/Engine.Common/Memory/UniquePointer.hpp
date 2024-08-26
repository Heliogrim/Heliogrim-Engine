#pragma once

#include <memory>
#include <Engine.Pedantic/Into/IntoImpl.hpp>

#include "../__macro.hpp"

namespace hg {
	template <typename Ty_, class Dy_ = ::std::default_delete<Ty_>>
	using UniquePtr = std::unique_ptr<Ty_, Dy_>;
}

/**/

namespace hg {
	template <typename To_, typename From_>
	struct into_impl<std::unique_ptr<To_>, std::unique_ptr<From_>> {
		[[nodiscard]] constexpr std::unique_ptr<To_> operator()(std::unique_ptr<From_>&& obj_) noexcept {
			return std::unique_ptr<To_> { std::move(obj_) };
		}
	};
}
