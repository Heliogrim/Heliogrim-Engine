#include "Image.hpp"

using namespace ember::engine::assets;
using namespace ember;

Image::Image(cref<asset_guid> guid_) :
    Data(guid_, Image::type_id) {}

Image::Image(cref<asset_guid> guid_, cref<string> url_) :
    Data(guid_, Image::type_id),
    _url(url_) {}
