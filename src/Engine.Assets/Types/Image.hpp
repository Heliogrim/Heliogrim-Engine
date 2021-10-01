#pragma once

#include <Engine.Common/String.hpp>

#include "Data.hpp"
#include "ImageLayout.hpp"

namespace ember::engine::assets {
    class Image :
        public Data<ImageLayout> {

    protected:
        Image(cref<asset_guid> guid_);

    private:
        string _url;
    };
}
