#include "TextureAsset.hpp"

#include <Engine.Serialization/Layout/DataLayout.hpp>

using namespace hg::engine::assets;

namespace hg::engine::serialization {
    template <>
    void DataLayout<TextureAsset>::describe() {

        using namespace ::hg::engine::serialization::layout;

        const auto guidLayout = make_sptr<DataLayout<Guid>>();
        guidLayout->reflect().storeType<AssetGuid>();
        guidLayout->describe();

        const auto assetNameLayout = make_sptr<DataLayout<u8>>();
        assetNameLayout->reflect().storeType<u8>();
        assetNameLayout->describe();

        /**/
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(TextureAsset, _guid));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(TextureAsset, _type));
        defineSlice<string>(offsetof(TextureAsset, _assetName), assetNameLayout);

        /**/
        defineObject(offsetof(TextureAsset, _baseImage), guidLayout);
        defineSlice<Vector<AssetGuid>>(offsetof(TextureAsset, _images), guidLayout);

        #if USE_SPAN_LAYOUT
        defineSpan(offsetof(Texture, _extent), sizeof(TextureAsset::_extent));
        #else
        defineValue<LayoutDefineValueType::eUInt32>(offsetof(TextureAsset, _extent.x));
        defineValue<LayoutDefineValueType::eUInt32>(offsetof(TextureAsset, _extent.y));
        defineValue<LayoutDefineValueType::eUInt32>(offsetof(TextureAsset, _extent.z));
        #endif

        defineValue<LayoutDefineValueType::eUInt8>(offsetof(TextureAsset, _format));
        defineValue<LayoutDefineValueType::eUInt32>(offsetof(TextureAsset, _mipLevel));
        defineValue<LayoutDefineValueType::eUInt8>(offsetof(TextureAsset, _textureType));
    }
}
