#include "AssetGuid.hpp"

#include <Engine.Serialization/Layout/DataLayout.hpp>

using namespace ember;

namespace ember::engine::serialization {
    template <>
    void DataLayout<asset_guid>::describe() {

        using namespace ::ember::engine::serialization::layout;

        defineValue<LayoutDefineValueType::eUInt32>(offsetof(asset_guid, pre));
        defineValue<LayoutDefineValueType::eUInt16>(offsetof(asset_guid, c0));
        defineValue<LayoutDefineValueType::eUInt16>(offsetof(asset_guid, c1));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(asset_guid, post));
    }
}
