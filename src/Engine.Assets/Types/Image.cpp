#include "Image.hpp"

using namespace ember::engine::assets;
using namespace ember;

Image::Image(cref<asset_guid> guid_) :
    Data(guid_, Image::type_id) {}

Image::Image(cref<asset_guid> guid_, mref<Vector<Url>> sources_) :
    Data(guid_, Image::type_id),
    _sources(_STD move(sources_)) {}
