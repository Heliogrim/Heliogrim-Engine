#include <stddef.h>
#include <Engine.Serialization/Layout/DataLayout.hpp>

#include "AssetGuid.hpp"

using namespace hg;

namespace hg::engine::serialization {
    template <>
    void DataLayout<AssetGuid>::describe() {

        using namespace ::hg::engine::serialization::layout;

        defineValue<LayoutDefineValueType::eUInt32>(offsetof(AssetGuid, pre));
        defineValue<LayoutDefineValueType::eUInt16>(offsetof(AssetGuid, c0));
        defineValue<LayoutDefineValueType::eUInt16>(offsetof(AssetGuid, c1));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(AssetGuid, post));
    }
}
