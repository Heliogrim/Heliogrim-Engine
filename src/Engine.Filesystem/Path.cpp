#include "Path.hpp"

#include <Engine.Common/Types.hpp>

using namespace hg::engine::fs;
using namespace hg;

Path::Path(mref<this_type> other_) noexcept :
    _value(_STD move(other_._value)) {}

Path::Path(cref<this_type> other_) :
    _value(other_._value) {}

Path::Path(cref<std_fs_path> path_) :
    _value(path_) {}

Path::Path(mref<std_fs_path> path_) :
    _value(_STD move(path_)) {}

Path::Path(mref<hg::string> str_) :
    _value(_STD move(str_)) {}

Path::Path(cref<hg::string> str_) :
    _value(str_) {}

Path::Path(string_view view_) :
    _value(view_) {}

ref<Path::this_type> Path::operator=(mref<this_type> other_) {
    if (_STD addressof(other_) != this) {
        _value = _STD move(other_._value);
    }
    return *this;
}

ref<Path::this_type> Path::operator=(cref<this_type> other_) {
    if (_STD addressof(other_) != this) {
        _value = other_._value;
    }
    return *this;
}

bool Path::hasName() const noexcept {
    // TODO:
    return not empty();
}

string Path::name() const {
    // TODO:
    const auto& val = _value.native();
    auto ls = val.find_last_of(std_fs_path::preferred_separator);

    if (ls == std_fs_path::string_type::npos) {

        if (empty()) {
            return {};
        }

        ls = 0ui64;
    }

    /**/

    const _STD basic_string_view<std_fs_path::value_type> subView { &(val.c_str()[ls]), val.size() - ls };
    ::hg::string tmp(val.size() - ls, 0);

    _STD transform(
        subView.begin(),
        subView.end(),
        tmp.begin(),
        [](wchar_t c) {
            return static_cast<char>(c);
        }
    );

    return tmp;
}

bool Path::hasParent() const noexcept {
    return _value.has_parent_path();
}

Path::this_type Path::parentPath() const {
    return this_type { _value.parent_path() };
}

string Path::parentName() const {
    return parentPath().name();
}

bool Path::empty() const noexcept {
    return _value.empty();
}

u32 Path::segments() const noexcept {
    const auto& val = _value.native();
    return _STD count(val.begin(), val.end(), std_fs_path::preferred_separator);
}

ref<Path::this_type> Path::push(cref<this_type> path_) {
    _value /= path_._value;
    return *this;
}

ref<Path::this_type> Path::push(mref<this_type> path_) {
    _value /= path_._value;
    return *this;
}

ref<Path::this_type> Path::pop() {
    *this = parentPath();
    return *this;
}

ref<Path::this_type> Path::pop(u32 count_) {

    this_type tmp { *this };
    for (; count_ > 0; --count_) {
        tmp.pop();
    }

    *this = tmp;
    return *this;
}

bool Path::contains(cref<this_type> path_) const {

    const auto relSize = _value.native().size();
    if (relSize >= path_._value.native().size()) {
        return false;
    }

    if (not _value.empty() && path_._value.native().at(relSize) != std_fs_path::preferred_separator) {
        return false;
    }

    return _value.native() == _STD basic_string_view<std_fs_path::value_type> {
        path_._value.native().c_str(), relSize
    };
}

bool Path::equals(cref<this_type> path_) const {
    return _value == path_._value;
}

_STD strong_ordering Path::compareLexical(cref<this_type> other_) const {
    return _value <=> other_._value;
}

bool Path::operator==(cref<this_type> other_) const {
    return equals(other_);
}

bool Path::operator!=(cref<this_type> other_) const {
    return not equals(other_);
}

string Path::string() const {
    return _value.string();
}

Path::std_fs_path Path::stdFsPath() const {
    return _value;
}

Path::operator std::string() const {
    return string();
}

Path::operator std::filesystem::path() const {
    return stdFsPath();
}
