#pragma once

#include <Engine.Common/String.hpp>

#include "Data.hpp"
#include "ImageLayout.hpp"

namespace ember::engine::assets {
    class Image :
        public Data<ImageLayout> {
    public:
        inline static const asset_type_id type_id { "Image"_typeId };

    protected:
        Image(cref<asset_guid> guid_);

    public:
        Image(cref<asset_guid> guid_, cref<string> url_);

    private:
        string _url;
    };
}
