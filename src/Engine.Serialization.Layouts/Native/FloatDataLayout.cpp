#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace hg::engine::serialization {
    template <>
    void DataLayout<float>::describe() {
        defineValue<layout::LayoutDefineValueType::eFloat>(0);
    }
}
