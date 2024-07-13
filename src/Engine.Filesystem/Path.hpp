#pragma once

#include <filesystem>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::fs {
	class Path {
	public:
		using this_type = Path;

		using std_fs_path = std::filesystem::path;

		static constexpr const char* separator = "/";

	public:
		constexpr Path() = default;

		constexpr Path(mref<this_type> other_) noexcept :
			_value(std::move(other_._value)) {}

		constexpr Path(cref<this_type> other_) = default;

		explicit Path(cref<std_fs_path> path_);

		explicit Path(mref<std_fs_path> path_);

		Path(mref<string> str_);

		Path(cref<string> str_);

		Path(string_view view_);

		~Path() = default;

	private:
		std::filesystem::path _value;

	public:
		constexpr ref<this_type> operator=(mref<this_type> other_) noexcept = default;

		constexpr ref<this_type> operator=(cref<this_type> other_) noexcept = default;

	public:
		[[nodiscard]] bool hasName() const noexcept;

		[[nodiscard]] string name() const;

	public:
		[[nodiscard]] bool hasParent() const noexcept;

		[[nodiscard]] this_type parentPath() const;

		[[nodiscard]] string parentName() const;

	public:
		[[nodiscard]] bool empty() const noexcept;

		[[nodiscard]] u32 segments() const noexcept;

	public:
		ref<this_type> push(cref<this_type> path_);

		ref<this_type> push(mref<this_type> path_);

		ref<this_type> pop();

		ref<this_type> pop(const u32 count_);

	public:
		[[nodiscard]] bool contains(cref<this_type> path_) const;

		template <class Type_ = Path>
		[[nodiscard]] bool isSubPath(cref<Type_> check_) const {
			return contains(check_);
		}

		[[nodiscard]] bool equals(cref<this_type> path_) const;

		template <class Type_ = Path>
		[[nodiscard]] bool equivalent(cref<Type_> check_) const {
			return equals(check_);
		}

		[[nodiscard]] std::strong_ordering compareLexical(cref<this_type> other_) const;

	public:
		[[nodiscard]] bool operator==(cref<this_type> other_) const;

		[[nodiscard]] bool operator!=(cref<this_type> other_) const;

		template <class Type_ = Path>
		[[nodiscard]] std::strong_ordering operator<=>(cref<Type_> other_) const {
			return compareLexical(other_);
		}

	public:
		[[nodiscard]] string string() const;

		[[nodiscard]] std_fs_path stdFsPath() const;

	public:
		[[nodiscard]] operator ::hg::string() const;

		[[nodiscard]] operator std_fs_path() const;
	};
}
