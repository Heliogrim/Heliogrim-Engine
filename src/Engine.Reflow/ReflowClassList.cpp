#include "ReflowClassList.hpp"

#include <ranges>

using namespace hg::engine::reflow;
using namespace hg;

void ReflowClassList::add(StringView className_) {
	if (std::ranges::find_if(
		*this,
		[&](const value_type& entry_) {
			return entry_.is<StringView>() ? entry_.as<StringView>() == className_ : entry_.as<String>() == className_;
		}
	) != Vector::end()) {
		return;
	}

	Vector::emplace_back(::hg::move(className_));
}

void ReflowClassList::drop(StringView className_) {
	std::erase_if(
		*this,
		[&](const value_type& entry_) {
			return entry_.is<StringView>() ? entry_.as<StringView>() == className_ : entry_.as<String>() == className_;
		}
	);
}
