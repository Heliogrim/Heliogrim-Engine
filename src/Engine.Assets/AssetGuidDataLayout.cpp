#include "AssetGuid.hpp"

#include <Engine.Serialization/Layout/DataLayout.hpp>

using namespace ember;

namespace ember::engine::serialization {

    template <>
    void DataLayout<asset_guid>::describe() {

        using namespace ::ember::engine::serialization::layout;

        defineValue<LayoutDefineValueType::eUInt16>(offsetof(asset_guid, pred));
        defineValue<LayoutDefineValueType::eUInt8>(offsetof(asset_guid, b1));
        defineValue<LayoutDefineValueType::eUInt8>(offsetof(asset_guid, b2));
        defineValue<LayoutDefineValueType::eUInt32>(offsetof(asset_guid, post));
    }

}
