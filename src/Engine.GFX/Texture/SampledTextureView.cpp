#include "SampledTextureView.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SampledTextureView::SampledTextureView(
    mref<nmpt<const TextureLikeObject>> tlo_,
    mref<nmpt<const TextureSampler>> so_
) noexcept :
    _tlo(_STD move(tlo_)),
    _so(_STD move(so_)) {}

SampledTextureView::~SampledTextureView() noexcept = default;

nmpt<const TextureLikeObject> SampledTextureView::object() const noexcept {
    return _tlo;
}

nmpt<const TextureSampler> SampledTextureView::samplerObject() const noexcept {
    return _so;
}
