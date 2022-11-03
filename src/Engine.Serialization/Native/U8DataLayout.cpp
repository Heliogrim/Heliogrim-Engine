#include "../Layout/DataLayout.hpp"

namespace ember::engine::serialization {

    template <>
    void DataLayout<unsigned char>::describe() {
        defineValue<layout::LayoutDefineValueType::eUInt8>(0);
    }

}
