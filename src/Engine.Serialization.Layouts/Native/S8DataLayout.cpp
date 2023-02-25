#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace ember::engine::serialization {
    template <>
    void DataLayout<s8>::describe() {
        defineValue<layout::LayoutDefineValueType::eInt8>(0);
    }
}
