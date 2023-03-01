#pragma once

#include <memory>

namespace hg {
    template <typename Type_>
    using SharedFromThis = _STD enable_shared_from_this<Type_>;
}
