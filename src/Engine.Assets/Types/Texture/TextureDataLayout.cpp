#include "Texture.hpp"

#include <Engine.Serialization/Layout/DataLayout.hpp>

using namespace ember::engine::assets;

namespace ember::engine::serialization {

    template <>
    void DataLayout<Texture>::describe() {

        using namespace ::ember::engine::serialization::layout;

        const auto assetGuidLayout = make_sptr<DataLayout<asset_guid>>();
        assetGuidLayout->reflect().storeType<asset_guid>();
        assetGuidLayout->describe();

        /**/
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(Texture, _guid));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(Texture, _type));

        /**/
        defineObject(offsetof(Texture, _baseImage), assetGuidLayout);
        defineSlice<Vector<asset_guid>>(offsetof(Texture, _images), assetGuidLayout);

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
