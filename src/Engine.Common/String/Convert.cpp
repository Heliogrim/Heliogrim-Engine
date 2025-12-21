#include "Convert.hpp"

#include <stdlib.h>
#include <Engine.Asserts/Todo.hpp>

using namespace hg;

#ifdef WIN32

std::wstring hg::stringToWString(ref<const std::string> string_) {
	auto result = std::wstring {};
	result.resize(string_.size());

	auto size = std::size_t {};
	::mbstowcs_s(&size, result.data(), result.size() + 1uLL, string_.c_str(), string_.size());
	return result;
}

#else

std::wstring hg::stringToWString(ref<const std::string> string_) {
	::hg::todo_panic();
}

#endif