#include "VkSurfaceSwapchain.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Engine.GFX/API/VkTranslate.hpp"
#include "Engine.GFX/Command/CommandQueue.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"

using namespace hg::engine::gfx;
using namespace hg;

/**
 *
 */

math::uivec2 clampExtent(math::uivec2 extent_, cref<vk::SurfaceCapabilitiesKHR> capabilities_) noexcept;

vk::PresentModeKHR selectPresentMode(cref<Vector<vk::PresentModeKHR>> modes_) noexcept;

void pretransform(cref<sptr<Device>> device_, cref<Vector<SwapchainImage>> textures_);

/**
 *
 */

VkSurfaceSwapchain::VkSurfaceSwapchain(const nmpt<Surface> owner_) :
	Swapchain(),
	_signals(),
	_vkSwapchain(),
	_surface(owner_) {}

VkSurfaceSwapchain::~VkSurfaceSwapchain() {
	destroy();
}

void VkSurfaceSwapchain::setup(cref<sptr<Device>> device_) {

	::hg::assertrt(device_.get());

	/**/
	Swapchain::setup(device_);

	/**/

	assert(/*_surface*/true);

	/**/

	const auto capabilities = _device->vkPhysicalDevice().getSurfaceCapabilitiesKHR(*_surface);

	_extent = clampExtent({ capabilities.currentExtent.width, capabilities.currentExtent.height }, capabilities);
	const auto mode = selectPresentMode(_device->vkPhysicalDevice().getSurfacePresentModesKHR(*_surface));

	_format = _surface->getImageFormat(*_device);

	vk::SurfaceTransformFlagBitsKHR surfaceTransform;
	if (capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
		surfaceTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	} else {
		surfaceTransform = capabilities.currentTransform;
	}

	u32 length = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && length > capabilities.maxImageCount) {
		length = capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR sci {
		vk::SwapchainCreateFlagsKHR(),
		*_surface,
		length,
		api::vkTranslateFormat(_format),
		vk::ColorSpaceKHR::eSrgbNonlinear,
		vk::Extent2D { _extent.x, _extent.y },
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		0,
		nullptr,
		surfaceTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		mode,
		VK_TRUE,
		nullptr
	};

	if (capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc) {
		sci.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
	}

	if (capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst) {
		sci.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;
	}

	if (capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eSampled) {
		sci.imageUsage |= vk::ImageUsageFlagBits::eSampled;
	}

	_vkSwapchain = _device->vkDevice().createSwapchainKHR(sci);
	::hg::assertd(_vkSwapchain);

	const auto swapImages = _device->vkDevice().getSwapchainImagesKHR(_vkSwapchain);
	_images.resize(swapImages.size(), {});

	for (u32 i = 0; i < swapImages.size(); ++i) {
		/**
		 *
		 */
		_images[i].image = make_smr<Texture>();
		auto& texture = _images[i].image;

		assert(texture);

		/**
		 *
		 */
		texture->type() = TextureType::e2d;
		texture->format() = _format;
		texture->extent() = { _extent, 1uL };
		texture->mipLevels() = 1uL;
		texture->buffer()._vkAspect = vk::ImageAspectFlagBits::eColor;
		texture->buffer().vkDevice() = _device->vkDevice();
		texture->buffer().image() = swapImages[i];

		TextureFactory::get()->buildView(*texture);

		assert(texture->vkView());
		assert(texture->buffer().image());
	}

	/**
	 *
	 */
	pretransform(_device, _images);
}

void VkSurfaceSwapchain::destroy() {

	/**/
	for (auto&& signal : _signals) {
		_device->vkDevice().destroySemaphore(signal);
	}
	_signals.clear();

	/**
	 * We are must not destroy/delete the swapchain acquired images
	 */
	for (auto& entry : _images) {
		entry.image->buffer().image() = VK_NULL_HANDLE;
	}

	/**
	 *
	 */
	for (auto& entry : _images) {
		/**/
		entry.image->destroy();
		/**/
		if (entry.readySignal) {
			_device->vkDevice().destroySemaphore(entry.readySignal);
		}
	}

	_images.clear();

	_device->vkDevice().destroySwapchainKHR(_vkSwapchain);
	_vkSwapchain = VK_NULL_HANDLE;
}

s64 VkSurfaceSwapchain::imageCount() const noexcept {
	return _images.size();
}

bool VkSurfaceSwapchain::acquireNext(ref<s64> idx_, ref<smr<Texture>> image_, ref<vk::Semaphore> signal_) {

	const auto nxtSig { nextSignal() };

	u32 nextIdx { ~0uL };
	// TODO: Handle next result while acquiring next image
	[[maybe_unused]] auto nextResult {
		_device->vkDevice().acquireNextImageKHR(
			_vkSwapchain,
			UINT64_MAX,
			nxtSig,
			VK_NULL_HANDLE,
			&nextIdx
		)
	};

	/**
	 * Store local
	 */
	restoreSignal(_images[nextIdx].readySignal);

	_images[nextIdx].readySignal = nxtSig;
	_images[nextIdx].presentWaits.clear();

	/**
	 * Store output
	 */
	idx_ = static_cast<s64>(nextIdx);
	image_ = clone(_images[nextIdx].image);
	signal_ = nxtSig;

	return true;
}

vk::Result VkSurfaceSwapchain::presentNext(u64 idx_) {

	const u32 idx { static_cast<u32>(idx_) };
	ref<SwapchainImage> image { _images[idx_] };

	const vk::PresentInfoKHR info {
		static_cast<u32>(image.presentWaits.size()),
		image.presentWaits.data(),
		1uL,
		&_vkSwapchain,
		&idx
	};

	const auto vkResult {
		_device->graphicsQueue()->vkQueue().presentKHR(info)
	};

	return vkResult;
}

vk::Result VkSurfaceSwapchain::presentNext(u64 idx_, cref<Vector<vk::Semaphore>> waits_) {

	const u32 idx { static_cast<u32>(idx_) };
	ref<SwapchainImage> image { _images[idx_] };

	/**/
	image.presentWaits.insert(image.presentWaits.end(), waits_.begin(), waits_.end());

	const vk::PresentInfoKHR info {
		static_cast<u32>(image.presentWaits.size()),
		image.presentWaits.data(),
		1uL,
		&_vkSwapchain,
		&idx
	};

	const auto vkResult {
		_device->graphicsQueue()->vkQueue().presentKHR(info)
	};

	return vkResult;
}

bool VkSurfaceSwapchain::consumeNext(
	ref<smr<Texture>> image_,
	ref<vk::Semaphore> signal_,
	ref<Vector<vk::Semaphore>> waits_
) {
	return false;
}

vk::Semaphore VkSurfaceSwapchain::nextSignal() {

	if (!_signals.empty()) {
		auto signal { _signals.back() };
		_signals.pop_back();
		return signal;
	}

	vk::SemaphoreCreateInfo info {};
	const auto signal { _device->vkDevice().createSemaphore(info) };
	return signal;
}

void VkSurfaceSwapchain::restoreSignal(const vk::Semaphore signal_) {

	if (!signal_) {
		return;
	}

	_signals.push_back(signal_);
}

/**
 *
 */

#include <Engine.GFX/Device/Device.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>

math::uivec2 clampExtent(math::uivec2 extent_, cref<vk::SurfaceCapabilitiesKHR> capabilities_) noexcept {

	extent_ = math::compMin(
		extent_,
		math::uivec2 { capabilities_.maxImageExtent.width, capabilities_.maxImageExtent.height }
	);

	extent_ = math::compMax(
		extent_,
		math::uivec2 { capabilities_.minImageExtent.width, capabilities_.minImageExtent.height }
	);

	// extent_ = math::compMax(extent_, math::uivec2 { 1uL });

	return extent_;
}

vk::PresentModeKHR selectPresentMode(cref<Vector<vk::PresentModeKHR>> modes_) noexcept {
	vk::PresentModeKHR pm { vk::PresentModeKHR::eFifo };

	for (u32 i = 0; i < modes_.size(); ++i) {
		if (modes_[i] == vk::PresentModeKHR::eMailbox) {
			pm = vk::PresentModeKHR::eMailbox;
			break;
		}

		if (modes_[i] == vk::PresentModeKHR::eImmediate) {
			pm = vk::PresentModeKHR::eImmediate;
		}
	}

	return pm;
}

void pretransform(cref<sptr<Device>> device_, cref<Vector<SwapchainImage>> textures_) {

	Vector<vk::ImageMemoryBarrier> imgBarriers {};
	for (const auto& entry : textures_) {

		if (isDepthFormat(entry.image->format()) || isStencilFormat(entry.image->format())) {
			continue;
		}

		imgBarriers.push_back(
			{
				vk::AccessFlags {},
				vk::AccessFlags {},
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				entry.image->buffer().image(),
				vk::ImageSubresourceRange {
					vk::ImageAspectFlagBits::eColor,
					0,
					entry.image->mipLevels(),
					0,
					entry.image->layer()
				}
			}
		);
	}

	const auto pool = device_->graphicsQueue()->pool();
	pool->lck().acquire();
	CommandBuffer cmd = pool->make();
	cmd.begin();

	/**
	 * Transform
	 */
	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlagBits::eAllCommands,
		vk::DependencyFlags {},
		0,
		nullptr,
		0,
		nullptr,
		static_cast<u32>(imgBarriers.size()),
		imgBarriers.data()
	);

	cmd.end();
	cmd.submitWait();
	cmd.release();

	pool->lck().release();
}
