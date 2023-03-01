#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace hg::engine::serialization {
    template <>
    void DataLayout<s16>::describe() {
        defineValue<layout::LayoutDefineValueType::eInt16>(0);
    }
}
