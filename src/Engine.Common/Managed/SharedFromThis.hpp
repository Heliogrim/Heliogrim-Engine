#pragma once

#include <memory>

namespace ember {

    template <typename Type_>
    using SharedFromThis = _STD enable_shared_from_this<Type_>;

}