#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace hg::engine::serialization {
    template <>
    void DataLayout<s32>::describe() {
        defineValue<layout::LayoutDefineValueType::eInt32>(0);
    }
}
