#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Hash/Fnv-1a.hpp>

namespace hg {
	class CompileString {
	public:
		friend constexpr CompileString operator""_cs(const char*, std::size_t);

	private:
		constexpr CompileString(const char* begin_, std::size_t length_) :
			_cs(begin_),
			_length(length_) {}

	public:
		template <std::size_t NumChars_>
		constexpr CompileString(const char (&cs)[NumChars_]) :
			CompileString(&cs[0], /* Note: We drop the null terminator */ NumChars_ - 1) {}

		constexpr CompileString(const char* begin_, const char* end_) :
			CompileString(begin_, static_cast<std::size_t>(end_ - begin_)) {}

		[[nodiscard]] constexpr std::size_t length() const noexcept {
			return _length;
		}

		[[nodiscard]] constexpr std::size_t hash() const noexcept {
			return hash::fnv1a(string_view { _cs, _length });
		}

	public:
		[[nodiscard]] constexpr const char* begin() const noexcept {
			return _cs;
		}

		START_SUPPRESS_WARNINGS

		[[nodiscard]] constexpr const char* end() const noexcept {
			return _cs + _length;
		}

		STOP_SUPPRESS_WARNINGS

		[[nodiscard]] std::string str() const {
			return std::string { begin(), end() };
		}

	private:
		const char* _cs;
		std::size_t _length;
	};

	/**/

	constexpr CompileString operator""_cs(const char* str_, const std::size_t count_) {
		return CompileString { str_, count_ };
	}
}
