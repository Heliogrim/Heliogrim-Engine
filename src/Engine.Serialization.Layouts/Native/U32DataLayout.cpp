#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace ember::engine::serialization {
    template <>
    void DataLayout<u32>::describe() {
        defineValue<layout::LayoutDefineValueType::eUInt32>(0);
    }
}
