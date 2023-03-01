#include "Texture.hpp"

#include <Engine.Serialization/Layout/DataLayout.hpp>

using namespace hg::engine::assets;

namespace hg::engine::serialization {
    template <>
    void DataLayout<Texture>::describe() {

        using namespace ::hg::engine::serialization::layout;

        const auto guidLayout = make_sptr<DataLayout<Guid>>();
        guidLayout->reflect().storeType<asset_guid>();
        guidLayout->describe();

        const auto assetNameLayout = make_sptr<DataLayout<u8>>();
        assetNameLayout->reflect().storeType<u8>();
        assetNameLayout->describe();

        /**/
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(Texture, _guid));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(Texture, _type));
        defineSlice<string>(offsetof(Texture, _assetName), assetNameLayout);

        /**/
        defineObject(offsetof(Texture, _baseImage), guidLayout);
        defineSlice<Vector<asset_guid>>(offsetof(Texture, _images), guidLayout);

        #if USE_SPAN_LAYOUT
    defineSpan(offsetof(Texture, _extent), sizeof(Texture::_extent));
        #else
        defineValue<LayoutDefineValueType::eUInt32>(offsetof(Texture, _extent.x));
        defineValue<LayoutDefineValueType::eUInt32>(offsetof(Texture, _extent.y));
        defineValue<LayoutDefineValueType::eUInt32>(offsetof(Texture, _extent.z));
        #endif

        defineValue<LayoutDefineValueType::eUInt8>(offsetof(Texture, _format));
        defineValue<LayoutDefineValueType::eUInt32>(offsetof(Texture, _mipLevel));
        defineValue<LayoutDefineValueType::eUInt8>(offsetof(Texture, _textureType));
    }
}
