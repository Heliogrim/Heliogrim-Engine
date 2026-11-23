import Engine.Serialization;

namespace hg::engine::serialization {
    template <>
    void DataLayout<s64>::describe() {
        defineValue<layout::LayoutDefineValueType::eInt64>(0);
    }
}
