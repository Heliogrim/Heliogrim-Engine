import Engine.Serialization;

namespace hg::engine::serialization {
    template <>
    void DataLayout<double>::describe() {
        defineValue<layout::LayoutDefineValueType::eDouble>(0);
    }
}
