#include "StreamableAsset.hpp"

using namespace hg;

StreamableAsset::StreamableAsset(cref<asset_guid> guid_, cref<asset_type_id> typeId_) noexcept :
    Asset(
        guid_,
        typeId_,
        nullptr
    ) {}

StreamableAsset::~StreamableAsset() noexcept = default;
