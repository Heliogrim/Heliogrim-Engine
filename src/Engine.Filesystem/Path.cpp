#include "Path.hpp"

#include <Engine.Common/Types.hpp>

using namespace hg::engine::fs;
using namespace hg;

bool Path::hasParent() const noexcept {
	return std::filesystem::path { _value }.has_parent_path();
}

Path::this_type Path::parentPath() const {
	return Path { std::filesystem::path { _value }.parent_path() };
}

string Path::parentName() const {
	return parentPath().name();
}

ref<Path::this_type> Path::push(cref<this_type> path_) {
	auto tmp = std::filesystem::path { std::move(_value) };
	tmp /= path_._value;
	_value = tmp;
	return *this;
}

ref<Path::this_type> Path::push(mref<this_type> path_) {
	auto tmp = std::filesystem::path { std::move(_value) };
	tmp /= std::move(path_._value);
	_value = tmp;
	return *this;
}

ref<Path::this_type> Path::pop() {
	*this = parentPath();
	return *this;
}

ref<Path::this_type> Path::pop(u32 count_) {
	for (; count_ > 0; --count_) {
		this->pop();
	}
	return *this;
}
