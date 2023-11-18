#include "Framebuffer.hpp"

#include <Engine.GFX.Acc/Pass/VkGraphicsPass.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

#include "Engine.GFX/Texture/Texture.hpp"
#include "Engine.GFX/Texture/TextureView.hpp"
#include "Engine.GFX/Texture/VirtualTexture.hpp"
#include "Engine.GFX/Texture/VirtualTextureView.hpp"

using namespace hg::engine::gfx;
using namespace hg;

Framebuffer::Framebuffer(cref<sptr<Device>> device_) :
    _device(device_) { }

Framebuffer::Framebuffer(mref<Framebuffer> other_) noexcept :
    _attachments(_STD move(other_._attachments)),
    _extent(_STD exchange(other_._extent, math::uivec3 {})),
    _device(_STD move(other_._device)),
    _renderPass(_STD move(other_._renderPass)),
    _vkFramebuffer(_STD exchange(other_._vkFramebuffer, nullptr)) { }

Framebuffer::~Framebuffer() noexcept {
    if (_vkFramebuffer) {
        destroy();
    }
}

void Framebuffer::destroy() {
    _device->vkDevice().destroyFramebuffer(_vkFramebuffer);
    _vkFramebuffer = nullptr;

    /**
     * Cleanup Attachments
     */
    _attachments.clear();
    _attachments.shrink_to_fit();
}

void Framebuffer::setup() {

    assert(_device);
    assert(!_vkFramebuffer);

    assert(not _renderPass.empty());
    const auto* const renderPass = Cast<acc::VkGraphicsPass>(_renderPass.get());

    /**
     * Prepare
     */
    Vector<vk::ImageView> attachments {};

    for (const auto& entry : _attachments) {

        auto view = switchType<TextureLikeObject>(
            entry.get(),
            [](const ptr<Texture> obj_) {
                return obj_->vkView();
            },
            [](const ptr<TextureView> obj_) {
                return obj_->owner()->vkView();
            },
            [](const ptr<VirtualTexture> obj_) {
                return obj_->_vkImageView;
            },
            [](const ptr<VirtualTextureView> obj_) {
                return reinterpret_cast<VkImageView>(obj_->vkImageView());
            }
        );

        assert(view);
        attachments.push_back(view);
    }

    vk::FramebufferCreateInfo info {
        vk::FramebufferCreateFlags(),
        reinterpret_cast<VkRenderPass>(renderPass->_vkGraphicsPass),
        static_cast<u32>(attachments.size()),
        attachments.data(),
        _extent.x,
        _extent.y,
        _extent.z/* TODO: MIN(attachments.size(), _extent.z) */
    };

    /**
     * Create
     */
    _vkFramebuffer = _device->vkDevice().createFramebuffer(info);
}

void Framebuffer::addAttachment(mref<smr<TextureLikeObject>> textureLikeObject_) {
    _attachments.push_back(_STD move(textureLikeObject_));
}

cref<Vector<smr<TextureLikeObject>>> Framebuffer::attachments() const noexcept {
    return _attachments;
}

cref<math::uivec3> Framebuffer::extent() const noexcept {
    return _extent;
}

u32 Framebuffer::width() const noexcept {
    return _extent.x;
}

u32 Framebuffer::height() const noexcept {
    return _extent.y;
}

void Framebuffer::setExtent(cref<math::uivec3> extent_) noexcept {
    _extent = extent_;
}

cref<sptr<Device>> Framebuffer::device() const noexcept {
    return _device;
}

cref<smr<const acc::AccelerationPass>> Framebuffer::renderPass() const noexcept {
    return _renderPass;
}

void Framebuffer::setRenderPass(mref<smr<const acc::AccelerationPass>> renderPass_) {
    _renderPass = _STD move(renderPass_);
}

const vk::Framebuffer& Framebuffer::vkFramebuffer() const noexcept {
    return _vkFramebuffer;
}

Framebuffer::operator vk::Framebuffer() const noexcept {
    return _vkFramebuffer;
}
