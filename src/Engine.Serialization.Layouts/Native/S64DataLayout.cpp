#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace hg::engine::serialization {
    template <>
    void DataLayout<s64>::describe() {
        defineValue<layout::LayoutDefineValueType::eInt64>(0);
    }
}
