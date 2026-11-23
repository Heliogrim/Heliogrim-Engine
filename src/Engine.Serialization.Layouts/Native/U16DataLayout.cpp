import Engine.Serialization;

namespace hg::engine::serialization {
    template <>
    void DataLayout<u16>::describe() {
        defineValue<layout::LayoutDefineValueType::eUInt16>(0);
    }
}
