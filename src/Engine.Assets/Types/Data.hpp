#pragma once
#include "Asset.hpp"
#include "../AssetOutputStream.hpp"
#include "Engine.Assets/AssetInputStream.hpp"

namespace ember::engine::assets {
    template <typename Layout>
    class Data :
        public Asset {
    protected:
        Data(cref<asset_guid> guid_) :
            Asset(guid_) {}

    public:
        AssetOutputStream::reference_type serialize(AssetOutputStream::reference_type stream_);

        AssetInputStream::reference_type deserialize(AssetInputStream::reference_type stream_);
    };
}
