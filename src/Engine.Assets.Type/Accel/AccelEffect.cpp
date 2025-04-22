#include "AccelEffect.hpp"

using namespace hg::engine::assets;
using namespace hg;

AccelEffect::AccelEffect(mref<AssetGuid> guid_) :
	InheritMeta(std::move(guid_), AccelEffect::typeId) {}
