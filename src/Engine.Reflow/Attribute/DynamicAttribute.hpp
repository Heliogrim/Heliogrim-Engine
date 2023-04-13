#pragma once

#include "Attribute.hpp"

namespace hg::engine::reflow {
    template <typename ValueType_, template <typename> typename ParentType_ = Attribute>
    class DynamicAttribute :
        public ParentType_<ValueType_> { };
}
