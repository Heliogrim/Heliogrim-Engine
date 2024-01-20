#pragma once

#include <filesystem>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::fs {
    class Path {
    public:
        using this_type = Path;

        using std_fs_path = _STD filesystem::path;

        static constexpr const char* separator = "/";

    public:
        constexpr Path() = default;

        Path(mref<this_type> other_) noexcept;

        Path(cref<this_type> other_);

        explicit Path(cref<std_fs_path> path_);

        explicit Path(mref<std_fs_path> path_);

        Path(mref<string> str_);

        Path(cref<string> str_);

        Path(string_view view_);

        ~Path() = default;

    private:
        _STD filesystem::path _value;

    public:
        ref<this_type> operator=(mref<this_type> other_) noexcept;

        ref<this_type> operator=(cref<this_type> other_);

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

        [[nodiscard]] _STD strong_ordering compareLexical(cref<this_type> other_) const;

    public:
        [[nodiscard]] bool operator==(cref<this_type> other_) const;

        [[nodiscard]] bool operator!=(cref<this_type> other_) const;

        template <class Type_ = Path>
        [[nodiscard]] _STD strong_ordering operator<=>(cref<Type_> other_) const {
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
