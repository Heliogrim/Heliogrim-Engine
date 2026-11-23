#include <stddef.h>

import Engine.Common;
import Engine.Serialization;

namespace hg::engine::serialization {
    template <>
    void DataLayout<Guid>::describe() {
        defineValue<layout::LayoutDefineValueType::eUInt32>(offsetof(Guid, pre));
        defineValue<layout::LayoutDefineValueType::eUInt16>(offsetof(Guid, c0));
        defineValue<layout::LayoutDefineValueType::eUInt16>(offsetof(Guid, c1));
        defineValue<layout::LayoutDefineValueType::eUInt64>(offsetof(Guid, post));
    }
}
