import Engine.Serialization;

namespace hg::engine::serialization {
    template <>
    void DataLayout<u32>::describe() {
        defineValue<layout::LayoutDefineValueType::eUInt32>(0);
    }
}
