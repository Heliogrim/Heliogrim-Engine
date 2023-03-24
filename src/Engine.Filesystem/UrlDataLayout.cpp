#include "Url.hpp"

#include <Engine.Serialization/Layout/DataLayout.hpp>

namespace hg::engine::serialization {
    template <>
    void DataLayout<fs::Url>::describe() {

        using namespace ::hg::engine::serialization::layout;

        const auto u8Layout = make_sptr<DataLayout<u8>>();
        u8Layout->reflect().storeType<u8>();
        u8Layout->describe();

        /**/
        //defineSlice<string>(offsetof(fs::Url, _scheme), u8Layout);
        //defineSlice<string>(offsetof(fs::Url, _path), u8Layout);
    }
}
