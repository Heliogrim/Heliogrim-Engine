#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace ember::engine::serialization {

    template <>
    void DataLayout<float>::describe() {
        defineValue<layout::LayoutDefineValueType::eFloat>(0);
    }

}
