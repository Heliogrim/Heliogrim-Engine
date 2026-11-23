import Engine.Serialization;

namespace hg::engine::serialization {
    template <>
    void DataLayout<float>::describe() {
        defineValue<layout::LayoutDefineValueType::eFloat>(0);
    }
}
