#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace ember::engine::serialization {

    template <>
    void DataLayout<u16>::describe() {
        defineValue<layout::LayoutDefineValueType::eUInt16>(0);
    }

}
