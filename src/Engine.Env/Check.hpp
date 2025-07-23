#pragma once

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::env {
	enum class EnvProps {
		/**/
		eIsRuntime,
		eIsEditor,
		eIsShipment,
		/**/
		eIsWindows,
		/**/
		eIsArchX86,
		eIsArchArmv8,
		/**/
		eIsCompileDebug,
		eIsRuntimeDebug/* TODO: May get problematic as it related to a runtime setting. */,
		eIsDebug/* TODO: @see above */,
	};

	/**/

	template <EnvProps Property_>
	extern consteval bool check() noexcept;

	/**/

	template <>
	consteval bool check<EnvProps::eIsEditor>() noexcept {
		// TODO:
		return true;
	}

	template <>
	consteval bool check<EnvProps::eIsRuntime>() noexcept {
		constexpr auto ed = check<EnvProps::eIsEditor>();
		return not ed;
	}

	template <>
	consteval bool check<EnvProps::eIsShipment>() noexcept {
		// TODO:
		return false;
	}

	/**/

	template <>
	consteval bool check<EnvProps::eIsWindows>() noexcept {
		#ifdef _WIN32
		return true;
		#else
		return false;
		#endif
	}

	/**/

	template <>
	constexpr bool check<EnvProps::eIsArchX86>() noexcept {
		#if _MSC_VER && (defined(_M_X64) || defined(_M_AMD64))
		return true;
		#elif (defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64))
		return true;
		#else
		return false;
		#endif
	}

	template <>
	constexpr bool check<EnvProps::eIsArchArmv8>() noexcept {
		constexpr auto x86 = check<EnvProps::eIsArchX86>();
		return not x86;
	}

	/**/

	template <>
	consteval bool check<EnvProps::eIsCompileDebug>() noexcept {
		#if not defined(NDEBUG) || _DEBUG
		return true;
		#else
		return false;
		#endif
	}

	template <>
	constexpr bool check<EnvProps::eIsRuntimeDebug>() noexcept {
		#if (not defined(ENV_WITH_DEBUG)) or (defined(ENV_WITH_DEBUG) and ENV_WITH_DEBUG)
		// TODO: We need to query for a runtime option to check. ( Currently force debug status... )
		constexpr bool* runtime_debug = nullptr;
		return runtime_debug != nullptr;
		#else
		return false;
		#endif
	}

	template <>
	constexpr bool check<EnvProps::eIsDebug>() noexcept {
		return check<EnvProps::eIsCompileDebug>() || check<EnvProps::eIsRuntimeDebug>();
	}
}
