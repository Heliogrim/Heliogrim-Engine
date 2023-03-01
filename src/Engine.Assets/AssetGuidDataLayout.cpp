#include "AssetGuid.hpp"

#include <Engine.Serialization/Layout/DataLayout.hpp>

using namespace hg;

namespace hg::engine::serialization {
    template <>
    void DataLayout<asset_guid>::describe() {

        using namespace ::hg::engine::serialization::layout;

        defineValue<LayoutDefineValueType::eUInt32>(offsetof(asset_guid, pre));
        defineValue<LayoutDefineValueType::eUInt16>(offsetof(asset_guid, c0));
        defineValue<LayoutDefineValueType::eUInt16>(offsetof(asset_guid, c1));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(asset_guid, post));
    }
}
