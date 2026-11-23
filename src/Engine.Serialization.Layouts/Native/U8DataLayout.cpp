import Engine.Serialization;

namespace hg::engine::serialization {
    template <>
    void DataLayout<u8>::describe() {
        defineValue<layout::LayoutDefineValueType::eUInt8>(0);
    }
}
