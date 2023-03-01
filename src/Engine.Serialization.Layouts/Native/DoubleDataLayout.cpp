#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace hg::engine::serialization {
    template <>
    void DataLayout<double>::describe() {
        defineValue<layout::LayoutDefineValueType::eDouble>(0);
    }
}
