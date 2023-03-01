#pragma once

#include <string>
#include <string_view>

namespace hg {
    using cstr = const char*;
    using string = _STD string;
    using string_view = _STD string_view;

    using namespace ::std::literals::string_view_literals;
}
