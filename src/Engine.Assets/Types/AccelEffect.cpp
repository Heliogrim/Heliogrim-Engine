#include "AccelEffect.hpp"

using namespace hg::engine::assets;
using namespace hg;

AccelEffect::AccelEffect(mref<asset_guid> guid_) :
    InheritMeta(_STD move(guid_), AccelEffect::typeId) {}
