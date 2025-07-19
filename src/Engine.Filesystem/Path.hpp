#pragma once

#include <algorithm>
#include <filesystem>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::fs {
	class Path;

	/**/

	template <typename StringType_>
	struct Separator {
		constexpr static const char* value = "/";
	};

	template <>
	struct Separator<std::wstring> {
		constexpr static const wchar_t* value = L"\\";
	};

	/**/

	class Path {
	public:
		using this_type = Path;

		using string_type = std::filesystem::path::string_type;
		using separator_type = Separator<string_type>;

	public:
		constexpr Path() noexcept = default;

		constexpr Path(mref<this_type> other_) noexcept :
			_value(std::move(other_._value)) {}

		constexpr Path(cref<this_type> other_) = default;

		explicit constexpr Path(cref<std::filesystem::path> path_) noexcept :
			_value(path_.native()) {}

		explicit constexpr Path(mref<std::filesystem::path> path_) noexcept :
			_value(path_.native()) {}

		explicit constexpr Path(mref<string_type> str_) noexcept :
			_value(std::move(str_)) {}

		explicit constexpr Path(const std::basic_string_view<string_type::value_type> str_) noexcept :
			_value(str_) {}

		template <typename StringType_> requires (not std::is_same_v<StringType_, string_type>)
		explicit Path(StringType_&& str_) noexcept :
			_value(std::filesystem::path { std::forward<StringType_>(str_) }.native()) {}

		~Path() = default;

	private:
		string_type _value;

	public:
		constexpr ref<this_type> operator=(mref<this_type> other_) noexcept = default;

		constexpr ref<this_type> operator=(cref<this_type> other_) noexcept = default;

	public:
		[[nodiscard]] constexpr bool hasName() const noexcept {
			return not empty();
		}

		[[nodiscard]] constexpr string_type nativeName() const {

			if (empty()) {
				return {};
			}

			const size_t last = _value.find_last_of(separator_type::value);
			if (last == string_type::npos) {
				return _value;
			}

			return _value.substr(last, string_type::npos);
		}

		[[nodiscard]] constexpr String name() const {

			// TODO: Check whether we can skip conversion if string_type equals String
			const auto tmp = nativeName();
			auto result = String(tmp.size(), 0);
			// TODO: Replace with actual narrow conversion
			std::transform(
				tmp.begin(),
				tmp.end(),
				result.begin(),
				[](string_type::value_type ch_) { return static_cast<char>(ch_); }
			);
			return result;
		}

	public:
		[[nodiscard]] bool hasParent() const noexcept;

		[[nodiscard]] this_type parentPath() const;

		[[nodiscard]] string parentName() const;

	public:
		[[nodiscard]] constexpr ref<const string_type> native() const noexcept {
			return _value;
		}

		[[nodiscard]] Path normalized() const & {
			return Path { static_cast<ref<const std::filesystem::path>>(*this).lexically_normal() };
		}

		[[nodiscard]] Path normalized() && {
			return Path { static_cast<mref<std::filesystem::path>>(::hg::move(*this)).lexically_normal() };
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			return _value.empty();
		}

		[[nodiscard]] constexpr u32 segments() const noexcept {
			return static_cast<u32>(std::count(_value.begin(), _value.end(), *separator_type::value));
		}

	public:
		ref<this_type> push(cref<this_type> path_);

		ref<this_type> push(mref<this_type> path_);

		ref<this_type> pop();

		ref<this_type> pop(const u32 count_);

	public:
		[[nodiscard]] constexpr bool contains(cref<this_type> path_) const {

			const auto relSize = _value.size();
			if (relSize >= path_._value.size()) {
				return false;
			}

			if (
				not _value.empty() &&
				_value.at(relSize - 1uL) != *separator_type::value &&
				path_._value.at(relSize) != *separator_type::value
			) {
				return false;
			}

			return _value == std::basic_string_view<string_type::value_type> {
				path_._value.begin(), path_._value.begin() + static_cast<string_type::difference_type>(relSize)
			};
		}

		template <class Type_ = Path>
		[[nodiscard]] bool isSubPathOf(ref<const Type_> parentCandidate_) const {

			const auto relSize = _value.size();
			const auto baseSize = parentCandidate_._value.size();
			if (relSize == 0uL || relSize <= baseSize) {
				return false;
			}

			if (
				parentCandidate_._value.at(baseSize - 1uL) != *separator_type::value &&
				_value.at(baseSize) != *separator_type::value
			) {
				return false;
			}

			return parentCandidate_._value == _value.substr(0uL, baseSize);
		}

		[[nodiscard]] constexpr bool equals(cref<this_type> other_) const {
			return _value == other_._value;
		}

		template <class Type_ = Path>
		[[nodiscard]] bool equivalent(cref<Type_> check_) const {
			// TODO:
			return equals(check_);
		}

		[[nodiscard]] constexpr std::strong_ordering compareLexical(cref<this_type> other_) const {
			return _value <=> other_._value;
		}

	public:
		[[nodiscard]] bool operator==(cref<std::filesystem::path> path_) const {
			return _value == path_.native();
		}

		[[nodiscard]] constexpr bool operator==(cref<this_type> other_) const {
			return equals(other_);
		}

		[[nodiscard]] constexpr bool operator!=(cref<this_type> other_) const {
			return not equals(other_);
		}

		template <class Type_ = Path>
		[[nodiscard]] constexpr std::strong_ordering operator<=>(cref<Type_> other_) const {
			return compareLexical(other_);
		}

	public:
		template <class StringType_ = String> requires std::is_same_v<StringType_, String>
		[[nodiscard]] explicit operator StringType_() const requires (not std::same_as<string_type, StringType_>) {
			return std::filesystem::path { _value }.string();
		}

		[[nodiscard]] explicit constexpr operator string_type() const {
			return _value;
		}

		template <typename PathType_ = Path> requires (sizeof(PathType_) == sizeof(std::filesystem::path))
		[[nodiscard]] constexpr operator cref<std::filesystem::path>() const noexcept {
			return *reinterpret_cast<const ptr<const std::filesystem::path>>(
				static_cast<const ptr<const PathType_>>(this)
			);
		}

		template <typename PathType_ = Path> requires (sizeof(PathType_) != sizeof(std::filesystem::path))
		[[nodiscard]] operator std::filesystem::path() const & noexcept {
			return std::filesystem::path { static_cast<const ptr<const PathType_>>(this)->_value };
		}

		[[nodiscard]] operator std::filesystem::path() && noexcept {
			return std::filesystem::path { std::move(_value) };
		}
	};
}
