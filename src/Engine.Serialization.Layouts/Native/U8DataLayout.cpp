#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace hg::engine::serialization {
    template <>
    void DataLayout<u8>::describe() {
        defineValue<layout::LayoutDefineValueType::eUInt8>(0);
    }
}
