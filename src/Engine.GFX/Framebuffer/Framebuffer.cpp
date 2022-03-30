#include "Framebuffer.hpp"

using namespace ember::engine::gfx;
using namespace ember;

Framebuffer::Framebuffer(cref<sptr<Device>> device_) :
    _device(device_) { }

Framebuffer::Framebuffer(mref<Framebuffer> other_) noexcept :
    _attachments(_STD move(other_._attachments)),
    _extent(_STD exchange(other_._extent, math::uivec3 {})),
    _device(_STD move(other_._device)),
    _renderPass(_STD exchange(other_._renderPass, wptr<pipeline::ApiRenderPass>())),
    _vkFramebuffer(_STD exchange(other_._vkFramebuffer, nullptr)) { }

Framebuffer::~Framebuffer() noexcept {
    if (_vkFramebuffer) {
        destroy();
    }
}

void Framebuffer::destroy() {
    _device->vkDevice().destroyFramebuffer(_vkFramebuffer);
    _vkFramebuffer = nullptr;
}

void Framebuffer::setup() {

    assert(_device);
    assert(!_vkFramebuffer);

    assert(!_renderPass.expired());
    auto renderPass = _renderPass.lock();

    /**
     * Prepare
     */
    Vector<vk::ImageView> attachments {};

    for (const auto& entry : _attachments) {
        attachments.push_back(entry->vkView());
    }

    vk::FramebufferCreateInfo info {
        vk::FramebufferCreateFlags(),
        renderPass->vkRenderPass(),
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

cref<Vector<FramebufferAttachment>> Framebuffer::attachments() const noexcept {
    return _attachments;
}

ref<Vector<FramebufferAttachment>> Framebuffer::attachments() noexcept {
    return _attachments;
}

void Framebuffer::add(FramebufferAttachment&& attachment_) {
    _attachments.push_back(_STD forward<FramebufferAttachment>(attachment_));
}

void Framebuffer::add(cref<FramebufferAttachment> attachment_) {
    _attachments.push_back(attachment_);
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

cref<wptr<pipeline::ApiRenderPass>> Framebuffer::renderPass() const noexcept {
    return _renderPass;
}

void Framebuffer::setRenderPass(wptr<pipeline::ApiRenderPass> renderPass_) {
    _renderPass = renderPass_;
}

const vk::Framebuffer& Framebuffer::vkFramebuffer() const noexcept {
    return _vkFramebuffer;
}

Framebuffer::operator vk::Framebuffer() const noexcept {
    return _vkFramebuffer;
}
