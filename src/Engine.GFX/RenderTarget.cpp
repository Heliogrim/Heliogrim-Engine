#include "RenderTarget.hpp"

#include <Engine.Common/Discard.hpp>
#include <Engine.GFX.Render/RenderPass.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Command/CommandBatch.hpp"
#include "Command/CommandQueue.hpp"
#include "Device/Device.hpp"
#include "Surface/Surface.hpp"
#include "Swapchain/Swapchain.hpp"

using namespace hg::engine::render;
using namespace hg::engine::gfx;
using namespace hg;

/**/

[[nodiscard]] constexpr static auto next_index_from(u8 syncIdx_) noexcept {
	return 1 - syncIdx_;
}

[[nodiscard]] constexpr static auto sync_index_from(u8 syncIdx_) noexcept {
	return syncIdx_;
}

/**/

RenderTarget::RenderTarget(mref<sptr<Device>> device_, const nmpt<const render::Renderer> renderer_) :
	_device(std::move(device_)),
	_renderer(renderer_),
	_sceneView(),
	_renderPasses(),
	_chainSceneView(nullptr),
	_chainSceneViewMask(0u),
	_swapchain(nullptr),
	_surface(nullptr),
	_chainSwapChain(nullptr),
	_chainSwapChainMask(0u),
	_otfImage(),
	_otfFinish(),
	_otfCpuGpu(),
	_enforceCpuGpuSync(true),
	_onTheFlight(true),
	_syncIdx(0uL),
	_swapIdx(~0uL),
	_swapSignal(),
	_active(false) {}

RenderTarget::~RenderTarget() {
	tidy();
}

void RenderTarget::tidy() {

	const auto req { _onTheFlight ? 2uL : 1uL };

	/**/

	for (u32 i = 0; i < req; ++i) {

		auto& renderPass = *_renderPasses[i];

		// Release bound Render Targets
		// renderPass.unbindTarget(<<symbol>>);
		auto storedSceneColor = renderPass.unbindTarget(makeSceneColorSymbol());
		::hg::discard(::hg::move(storedSceneColor));

		// Release bound Scene View
		//renderPass.changeSceneView(clone(sceneView_));
		renderPass.unbindSceneView();
	}

	for (auto&& renderPass : _renderPasses) {
		_renderer->free(std::move(renderPass));
	}
	_renderPasses.clear();

	/**
	 *
	 */
	for (u32 i = 0; i < _otfImage.size(); ++i) {
		_device->vkDevice().destroy(_otfImage[i]);
		_otfImage[i] = nullptr;

		//_device->vkDevice().destroy(_otfFinish[i]);
		//_otfFinish[i] = nullptr;

		_device->vkDevice().destroy(_otfCpuGpu[i]);
		_otfCpuGpu[i] = nullptr;
	}

	_otfImage.clear();
	//_otfFinish.clear();
	_otfCpuGpu.clear();
}

sptr<Device> RenderTarget::device() const noexcept {
	return _device;
}

nmpt<const Renderer> RenderTarget::getRenderer() const noexcept {
	return _renderer;
}

concurrent::Future<nmpt<const scene::SceneView>> RenderTarget::transitionToSceneView(
	mref<smr<const scene::SceneView>> sceneView_
) {

	assert(_chainSceneView == nullptr);
	assert(_sceneView.get() != sceneView_.get());

	_chainSceneViewMask = 0u;
	for (u8 i = static_cast<u8>(_renderPasses.size()); i > 0u; --i) {
		_chainSceneViewMask |= (0x1u << (i - 1u));
	}

	_chainSceneView = make_uptr<concurrent::Promise<nmpt<const scene::SceneView>>>(
		concurrent::Promise<nmpt<const scene::SceneView>>(
			// Warning: We are explicitly expanding the lifetime of the previous object
			[prev = clone(_sceneView)] {
				return prev.get();
			}
		)
	);

	/**/

	_sceneView = std::move(sceneView_);
	return _chainSceneView->get();
}

smr<const scene::SceneView> RenderTarget::getSceneView() const noexcept {
	return clone(_sceneView);
}

Result<TransitionResult, std::runtime_error> RenderTarget::transitionToTarget(
	ref<Surface> surface_,
	FnRef<smr<Swapchain>(mref<smr<Swapchain>> prev_)> transitionFn_
) {
	/**
	 * Block render passes
	 */
	if (_enforceCpuGpuSync) {

		const auto condOtfCount = std::min<u8>(_onTheFlight ? 2u : 1u, _renderPasses.size());
		for (auto i = 0u; i < condOtfCount; ++i) {

			auto cpuGpuSync { _renderPasses[i]->unsafeSync() };
			if (cpuGpuSync) {
				auto result { _device->vkDevice().waitForFences(1, &cpuGpuSync, VK_TRUE, UINT64_MAX) };
				// Might fail when one was submitted or pipeline gets dumped/error state
				assert(result == vk::Result::eSuccess);
			}
		}
	}

	/**/

	_surface = nullptr;
	auto nextSwapchain = transitionFn_(::hg::move(_swapchain));
	::hg::assertd(static_cast<bool>(nextSwapchain));

	const auto reqImg = _onTheFlight ? 1uL : 2uL;
	if (nextSwapchain->imageCount() < reqImg) {
		IM_CORE_WARNF(
			"Tried to use render target with unsupported swapchain. (min. Img. `{}`, provided `{}`)",
			reqImg,
			nextSwapchain->imageCount()
		);
		return tl::make_unexpected(std::runtime_error("Failed to transition with unsupported swapchain."));
	}

	/**/

	u8 fullSwapChainMask = 0u;
	for (u8 i = static_cast<u8>(_renderPasses.size()); i > 0u; --i) {
		fullSwapChainMask |= (0x1u << (i - 1u));
	}

	if (_chainSwapChain != nullptr) {

		/* Check for possible forceful override of untouched transition */
		if (_chainSwapChainMask != fullSwapChainMask) {
			IM_CORE_WARN("Tried to override ongoing transition at render target.");
			return tl::make_unexpected(std::runtime_error("Failed to override transition state."));
		}

		_swapchain = std::move(nextSwapchain);
		_surface = std::addressof(surface_);

		// Attention: At this point the internal future state has some shared ownership
		return TransitionResult { _chainSwapChain->get(), _swapchain };
	}

	/**/

	_chainSwapChainMask = fullSwapChainMask;
	_chainSwapChain = make_uptr<concurrent::Promise<std::monostate>>(
		concurrent::Promise<std::monostate>(
			// Warning: We are explicitly expanding the lifetime of the previous object
			[] { return std::monostate {}; }
		)
	);

	/**/

	_swapchain = std::move(nextSwapchain);
	_surface = std::addressof(surface_);

	return TransitionResult { _chainSwapChain->get(), _swapchain };
}

Result<TransitionResult, std::runtime_error> RenderTarget::transitionToTarget(mref<smr<Swapchain>> nextSwapchain_) {

	::hg::assertd(static_cast<bool>(nextSwapchain_) && _swapchain != nextSwapchain_);

	const auto reqImg = _onTheFlight ? 1uL : 2uL;
	if (nextSwapchain_->imageCount() < reqImg) {
		IM_CORE_WARNF(
			"Tried to use render target with unsupported swapchain. (min. Img. `{}`, provided `{}`)",
			reqImg,
			nextSwapchain_->imageCount()
		);
		return tl::make_unexpected(std::runtime_error("Failed to transition with unsupported swapchain."));
	}

	/**/

	u8 fullSwapChainMask = 0u;
	for (u8 i = static_cast<u8>(_renderPasses.size()); i > 0u; --i) {
		fullSwapChainMask |= (0x1u << (i - 1u));
	}

	if (_chainSwapChain != nullptr) {

		/* Check for possible forceful override of untouched transition */
		if (_chainSwapChainMask != fullSwapChainMask) {
			IM_CORE_WARN("Tried to override ongoing transition at render target.");
			return tl::make_unexpected(std::runtime_error("Failed to override transition state."));
		}

		_swapchain = std::move(nextSwapchain_);
		_surface = nullptr;

		// Attention: At this point the internal future state has some shared ownership
		return TransitionResult { _chainSwapChain->get(), _swapchain };
	}

	/**/

	_chainSwapChainMask = fullSwapChainMask;
	_chainSwapChain = make_uptr<concurrent::Promise<std::monostate>>(
		concurrent::Promise<std::monostate>(
			// Warning: We are explicitly expanding the lifetime of the previous object
			[previousKeepalive = clone(_swapchain)] { return std::monostate {}; }
		)
	);

	/**/

	_swapchain = std::move(nextSwapchain_);
	_surface = nullptr;

	return TransitionResult { _chainSwapChain->get(), _swapchain };
}

smr<Swapchain> RenderTarget::getSwapChain() const noexcept {
	return clone(_swapchain);
}

u8 RenderTarget::supportedFramesAhead() const noexcept {
	if (_onTheFlight) {
		// Use 2 Sync Sets -> (1 Current + 1 Ahead) | (1 Previous + 1 Current)
		return 1u;
	}

	return 0u;
}

void RenderTarget::flipRenderSync() {
	// Fast flip-flop idx
	_syncIdx = 1u - _syncIdx;
}

bool RenderTarget::ready() const noexcept {
	return _device && _renderer && _swapchain && !_renderPasses.empty();
}

bool RenderTarget::active() const noexcept {
	return _active;
}

bool RenderTarget::setActive(const bool active_) {
	const auto changed { _active != active_ };
	_active = active_;
	return changed;
}

void RenderTarget::setup() {

	assert(_device);
	assert(_renderer);
	assert(_sceneView);
	assert(_swapchain);

	assert(_renderPasses.empty());

	/**/

	const auto otfCount = _onTheFlight ? 2u : 1u;

	if (_onTheFlight) {
		_otfImage.resize(otfCount);
		//_otfFinish.resize(req);
		_otfCpuGpu.resize(otfCount);

		const vk::SemaphoreCreateInfo sci {};
		const vk::FenceCreateInfo fci { vk::FenceCreateFlagBits::eSignaled };

		for (auto i = 0u; i < otfCount; ++i) {

			_otfImage[i] = _device->vkDevice().createSemaphore(sci);
			//_otfFinish[i] = _device->vkDevice().createSemaphore(sci);
			_otfCpuGpu[i] = _device->vkDevice().createFence(fci);

			assert(_otfImage[i]);
			//assert(_otfFinish[i]);
			assert(_otfCpuGpu[i]);
		}
	}

	/**/

	_renderPasses.reserve(otfCount);

	for (auto i = 0u; i < otfCount; ++i) {

		// TODO: Use contextual allocation
		_renderPasses.emplace_back(_renderer->allocate());
		auto& renderPass = *_renderPasses.back();

		// Store Scene View
		renderPass.changeSceneView(clone(_sceneView));

		// Store Render Targets
		// renderPass.bindTarget(<<symbol>>, <<texture>>);
		const auto bindResult = renderPass.bindTarget(makeSceneColorSymbol(), clone(_swapchain->at(i)));
		assert(bindResult);
	}

	/**/

	if (_chainSceneView) {
		(*_chainSceneView)();
		_chainSceneView.reset();
	}

	if (_chainSwapChain) {
		(*_chainSwapChain)();
		_chainSwapChain.reset();
	}
}

void RenderTarget::transitionImmediately() {

	const auto otfCount = _onTheFlight ? 2u : 1u;

	/**
	 * Block render passes
	 */
	if (_enforceCpuGpuSync) {

		for (auto i = 0u; i < otfCount; ++i) {

			auto cpuGpuSync { _renderPasses[i]->unsafeSync() };
			if (cpuGpuSync) {
				auto result { _device->vkDevice().waitForFences(1, &cpuGpuSync, VK_TRUE, UINT64_MAX) };
				// Might fail when one was submitted or pipeline gets dumped/error state
				assert(result == vk::Result::eSuccess);
			}
		}
	}

	/**/

	for (auto i = 0u; i < otfCount; ++i) {

		auto renderPass = std::move(_renderPasses[i]);

		// Conditional Update Scene View
		if (_chainSceneView != nullptr) {
			renderPass->changeSceneView(clone(_sceneView));
		}

		// Conditional Update Render Targets
		if (_chainSwapChain != nullptr) {
			// renderPass.bindTarget(<<symbol>>, <<texture>>);
			const auto previous = renderPass->unbindTarget(makeSceneColorSymbol());
			const auto bindResult = renderPass->bindTarget(makeSceneColorSymbol(), clone(_swapchain->at(i)));
			assert(bindResult);
		}

		_renderPasses[i] = _renderer->reallocate(std::move(renderPass)).first;
		assert(_renderPasses[i]);
	}

	/**/

	if (_chainSceneView) {
		(*_chainSceneView)();
		_chainSceneView.reset();
	}

	if (_chainSwapChain) {
		(*_chainSwapChain)();
		_chainSwapChain.reset();
	}
}

nmpt<RenderPass> RenderTarget::next() {
	/**
	 * Check for enforced sync behaviour
	 */
	if (_enforceCpuGpuSync) {
		auto cpuGpuSync { _renderPasses[sync_index_from(_syncIdx)]->unsafeSync() };

		if (cpuGpuSync) {
			auto result { _device->vkDevice().waitForFences(1, &cpuGpuSync, VK_TRUE, UINT64_MAX) };
			assert(result == vk::Result::eSuccess);
		}
	}

	/**
	 * Next Swapchain Image
	 */
	s64 nextSwapIdx = -1;
	smr<Texture> nextImage {};
	vk::Semaphore nextSignal {};

	const auto nextResult = _swapchain->acquireNext(nextSwapIdx, nextImage, nextSignal);

	/**
	 * Ensure swapchain complies our expectations
	 */
	if (not nextResult) {
		return nullptr;
	}

	if (!_onTheFlight) {
		/**
		 * Get Invocation / Await Last Render
		 */
		const auto& pass = _renderPasses[sync_index_from(_syncIdx)];
		if (!pass->await()) {
			IM_CORE_WARN("Failed to validate await state of render invocation.");
		}
	}

	_swapSignal = nextSignal;
	_swapIdx = static_cast<u8>(nextSwapIdx);

	/**/

	const auto nextRenderIdx = next_index_from(_syncIdx);
	auto* const renderPass = _renderPasses[nextRenderIdx].get();

	// Conditional Scene View Update
	if (_chainSceneViewMask & (1u << nextRenderIdx)) {
		renderPass->changeSceneView(clone(_sceneView));
		_chainSceneViewMask &= ~(1u << nextRenderIdx);
	}

	// Conditional Target Update
	if (_chainSwapChainMask & (1u << nextRenderIdx)) {
		// renderPass.bindTarget(<<symbol>>, <<texture>>);
		const auto previous = renderPass->unbindTarget(makeSceneColorSymbol());
		const auto bindResult = renderPass->bindTarget(makeSceneColorSymbol(), clone(_swapchain->at(nextSwapIdx)));
		assert(bindResult);
	}

	/**/

	return renderPass;
}

void RenderTarget::update() {

	const auto renderIdx = next_index_from(_syncIdx);
	auto& renderPass = _renderPasses[renderIdx];

	/**/

	if (_chainSwapChainMask & (1u << renderIdx)) {
		renderPass->clearTargetWaitSignals(makeSceneColorSymbol());
		_chainSwapChainMask &= ~(1u << renderIdx);
	}

	/**/

	if (_onTheFlight && _swapSignal) {
		renderPass->clearTargetWaitSignals(makeSceneColorSymbol());
		renderPass->addTargetWaitSignal(makeSceneColorSymbol(), _swapSignal);
	}

	if (_onTheFlight) {
		/*
		renderPass->getTargetReadySignals().clear();
		renderPass->getTargetReadySignals().push_back(_otfFinish[_syncIdx]);
		*/
	}

	auto updated = _renderer->update(std::move(renderPass));
	(*updated)();
	_renderPasses[renderIdx] = std::move(updated);

	/**/

	if (_chainSceneView && _chainSceneViewMask == 0u) {
		(*_chainSceneView)();
		_chainSceneView.reset();
	}

	if (_chainSwapChain && _chainSwapChainMask == 0u) {
		(*_chainSwapChain)();
		_chainSwapChain.reset();
	}
}

RenderEnqueueResult RenderTarget::finish(
	cref<Vector<vk::Semaphore>> waits_/* , cref<Vector<vk::Semaphore>> signals_ */
) {
	/**
	 * Combine Semaphores
	 */
	Vector<vk::Semaphore> waits {};
	waits.insert(waits.end(), waits_.begin(), waits_.end());

	if (_onTheFlight) {
		//waits.push_back(_otfFinish[_syncIdx]);
		const auto renderIdx = next_index_from(_syncIdx);
		_renderPasses[renderIdx]->enumerateTargetReadySignals(waits);
	}

	/*
	Vector<vk::Semaphore> signals {};
	signals.insert(signals.end(), signals_.begin(), signals_.end());
	 */

	/**
	 *
	 */
	const auto vkResult { _swapchain->presentNext(_swapIdx, waits) };
	RenderEnqueueResult result {};

	if (vkResult == vk::Result::eSuboptimalKHR || vkResult == vk::Result::eErrorOutOfDateKHR) {
		result = RenderEnqueueResult::eFailedChanged;
	} else if (vkResult != vk::Result::eSuccess) {
		result = RenderEnqueueResult::eFailed;
	}

	/**
	 *
	 */
	flipRenderSync();
	return result;
}
