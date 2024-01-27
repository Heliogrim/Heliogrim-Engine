#include "StreamableAsset.hpp"

using namespace hg;

StreamableAsset::StreamableAsset(mref<asset_guid> guid_, cref<asset_type_id> typeId_) noexcept :
    Asset(
        std::move(guid_),
        typeId_,
        nullptr
    ) {}

StreamableAsset::~StreamableAsset() noexcept = default;
