#include "Image.hpp"

#include <Engine.Serialization/Layout/DataLayout.hpp>

using namespace ember::engine::assets;

namespace ember::engine::serialization {

    template <>
    void DataLayout<Image>::describe() {

        using namespace ::ember::engine::serialization::layout;

        const auto assetGuidLayout = make_sptr<DataLayout<asset_guid>>();
        assetGuidLayout->reflect().storeType<asset_guid>();
        assetGuidLayout->describe();

        const auto urlLayout = make_sptr<DataLayout<Url>>();
        urlLayout->reflect().storeType<Url>();
        urlLayout->describe();

        /**/
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(Image, _guid));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(Image, _type));

        /**/
        defineSlice<Vector<Url>>(offsetof(Image, _sources), urlLayout);
    }

}
