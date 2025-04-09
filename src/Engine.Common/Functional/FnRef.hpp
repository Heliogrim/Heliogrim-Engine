#pragma once

#include <std23/function_ref.h>

namespace hg {
	template <typename Fn_, typename... Args_>
	using FnRef = std23::function_ref<Fn_, Args_...>;
}
