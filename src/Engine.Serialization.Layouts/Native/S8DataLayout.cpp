import Engine.Serialization;

namespace hg::engine::serialization {
    template <>
    void DataLayout<s8>::describe() {
        defineValue<layout::LayoutDefineValueType::eInt8>(0);
    }
}
