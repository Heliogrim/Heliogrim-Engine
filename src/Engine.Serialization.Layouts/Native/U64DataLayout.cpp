#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace hg::engine::serialization {
    template <>
    void DataLayout<u64>::describe() {
        defineValue<layout::LayoutDefineValueType::eUInt64>(0);
    }
}
