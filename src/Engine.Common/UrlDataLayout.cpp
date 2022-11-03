#include "Url.hpp"

#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace ember::engine::serialization {

    template <>
    void DataLayout<Url>::describe() {

        using namespace ::ember::engine::serialization::layout;

        const auto u8Layout = make_sptr<DataLayout<u8>>();
        u8Layout->reflect().storeType<u8>();
        u8Layout->describe();

        /**/
        defineSlice<string>(offsetof(Url, _scheme), u8Layout);
        defineSlice<string>(offsetof(Url, _user), u8Layout);
        defineSlice<string>(offsetof(Url, _password), u8Layout);
        defineSlice<string>(offsetof(Url, _host), u8Layout);
        defineValue<LayoutDefineValueType::eUInt16>(offsetof(Url, _port));
        defineSlice<string>(offsetof(Url, _path), u8Layout);
        defineSlice<string>(offsetof(Url, _query), u8Layout);
        defineSlice<string>(offsetof(Url, _fragment), u8Layout);
    }

}
