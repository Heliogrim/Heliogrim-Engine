#pragma once

#include <string>
#include <string_view>

namespace hg {
    using cstr = const char*;
    using string = std::string;
    using string_view = std::string_view;

    using String = string;
    using StringView = string_view;

    using namespace ::std::literals::string_view_literals;
}
