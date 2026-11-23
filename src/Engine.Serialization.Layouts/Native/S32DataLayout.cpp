import Engine.Serialization;

namespace hg::engine::serialization {
    template <>
    void DataLayout<s32>::describe() {
        defineValue<layout::LayoutDefineValueType::eInt32>(0);
    }
}
