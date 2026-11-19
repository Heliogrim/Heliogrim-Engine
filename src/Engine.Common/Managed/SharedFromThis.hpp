#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg {
    template <typename Type_>
    using SharedFromThis = std::enable_shared_from_this<Type_>;
}
