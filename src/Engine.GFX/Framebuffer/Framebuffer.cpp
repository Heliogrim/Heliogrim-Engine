#include "Framebuffer.hpp"

#include <utility>
#include <Engine.Accel.Pass/AccelerationPass.hpp>
#include <Engine.Accel.Pass/VkGraphicsPass.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

#include "Engine.GFX/Texture/SparseTexture.hpp"
#include "Engine.GFX/Texture/SparseTextureView.hpp"
#include "Engine.GFX/Texture/Texture.hpp"
#include "Engine.GFX/Texture/TextureView.hpp"

using namespace hg::engine::gfx;
using namespace hg;

Framebuffer::Framebuffer(cref<sptr<Device>> device_) :
	_attachments(),
	_extent(),
	_layer(1uL),
	_device(device_),
	_renderPass(nullptr),
	_vkFramebuffer(nullptr) {}

Framebuffer::Framebuffer(mref<Framebuffer> other_) noexcept :
	_attachments(std::move(other_._attachments)),
	_extent(std::exchange(other_._extent, math::uivec2 {})),
	_layer(std::exchange(other_._layer, 0uL)),
	_device(std::move(other_._device)),
	_renderPass(std::move(other_._renderPass)),
	_vkFramebuffer(std::exchange(other_._vkFramebuffer, nullptr)) {}

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
	const auto* const renderPass = Cast<accel::VkGraphicsPass>(_renderPass.get());

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
				if (obj_->_vkImageView != nullptr) {
					return vk::ImageView { reinterpret_cast<VkImageView>(obj_->_vkImageView) };
				}
				return obj_->owner()->vkView();
			},
			[](const ptr<SparseTexture> obj_) {
				return obj_->_vkImageView;
			},
			[](const ptr<SparseTextureView> obj_) {
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
		_layer
	};

	/**
	 * Create
	 */
	_vkFramebuffer = _device->vkDevice().createFramebuffer(info);
}

void Framebuffer::addAttachment(mref<smr<TextureLikeObject>> textureLikeObject_) {
	_attachments.push_back(std::move(textureLikeObject_));
}

cref<Vector<smr<TextureLikeObject>>> Framebuffer::attachments() const noexcept {
	return _attachments;
}

cref<math::uivec2> Framebuffer::extent() const noexcept {
	return _extent;
}

u32 Framebuffer::width() const noexcept {
	return _extent.x;
}

u32 Framebuffer::height() const noexcept {
	return _extent.y;
}

u32 Framebuffer::layer() const noexcept {
	return _layer;
}

void Framebuffer::setExtent(cref<math::uivec2> extent_) noexcept {
	_extent = extent_;
}

void Framebuffer::setLayer(const u32 layer_) noexcept {
	_layer = layer_;
}

cref<sptr<Device>> Framebuffer::device() const noexcept {
	return _device;
}

cref<smr<const engine::accel::AccelerationPass>> Framebuffer::renderPass() const noexcept {
	return _renderPass;
}

void Framebuffer::setRenderPass(mref<smr<const engine::accel::AccelerationPass>> renderPass_) {
	_renderPass = std::move(renderPass_);
}

const vk::Framebuffer& Framebuffer::vkFramebuffer() const noexcept {
	return _vkFramebuffer;
}

Framebuffer::operator vk::Framebuffer() const noexcept {
	return _vkFramebuffer;
}
