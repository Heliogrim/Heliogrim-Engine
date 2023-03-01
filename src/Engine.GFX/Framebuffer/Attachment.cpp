#include "Attachment.hpp"

#include <Engine.Common/Make.hpp>

using namespace hg::engine::gfx;
using namespace hg;

FramebufferAttachment::FramebufferAttachment(Texture&& texture_) noexcept :
    _texture(make_sptr<Texture>(_STD move(texture_))) {}

FramebufferAttachment::FramebufferAttachment(cref<sptr<Texture>> texture_) noexcept :
    _texture(texture_) {}

ptr<const Texture> FramebufferAttachment::operator->() const noexcept {
    return _texture.get();
}

ptr<Texture> FramebufferAttachment::operator->() noexcept {
    return _texture.get();
}

cref<sptr<Texture>> FramebufferAttachment::unwrapped() const noexcept {
    return _texture;
}

ref<sptr<Texture>> FramebufferAttachment::unwrapped() noexcept {
    return _texture;
}
