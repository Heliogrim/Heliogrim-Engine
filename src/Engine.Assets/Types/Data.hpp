#pragma once
#include "Asset.hpp"
#include "../AssetOutputStream.hpp"
#include "../AssetInputStream.hpp"

namespace ember::engine::assets {
    template <typename Layout_>
    class Data :
        public Asset {
    public:
        using this_type = Data<Layout_>;
        using layout_type = Layout_;

    protected:
        Data(cref<asset_guid> guid_, cref<asset_type_id> type_) :
            Asset(guid_, type_) {}

    public:
        AssetOutputStream::reference_type serialize(AssetOutputStream::reference_type stream_);

        AssetInputStream::reference_type deserialize(AssetInputStream::reference_type stream_);
    };
}
