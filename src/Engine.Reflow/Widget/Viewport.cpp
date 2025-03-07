#include "Viewport.hpp"

#include "Engine.Common/Math/Convertion.hpp"
#include "Engine.GFX/Swapchain/VkSwapchain.hpp"

#if TRUE
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Heliogrim/Actor/Camera/CameraActor.hpp>

#include "Engine.GFX/RenderTarget.hpp"
#endif

#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.GFX.Scene/RenderSceneManager.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Render.Scene.Model/CameraModel.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Scene/SceneBase.hpp>

#include "../Layout/Style.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Viewport::Viewport() :
	Widget(),
	attr(
		Attributes {
			.minWidth = { this, { ReflowUnitType::eAuto, 0.F } },
			.width = { this, { ReflowUnitType::eAuto, 0.F } },
			.maxWidth = { this, { ReflowUnitType::eAuto, 0.F } },
			.minHeight = { this, { ReflowUnitType::eAuto, 0.F } },
			.height = { this, { ReflowUnitType::eAuto, 0.F } },
			.maxHeight = { this, { ReflowUnitType::eAuto, 0.F } }
		}
	),
	_swapChain(nullptr),
	_renderTarget(nullptr),
	_currentSwapChainImage(),
	_currentImageSignal(nullptr),
	_currentImageWaits(),
	_renderer(),
	_cameraActor(nullptr),
	_uvs(
		{
			math::vec2 { 0.F, 0.F },
			math::vec2 { 1.F, 0.F },
			math::vec2 { 1.F, 1.F },
			math::vec2 { 0.F, 1.F }
		}
	),
	_viewSize(),
	_viewListen() {}

Viewport::~Viewport() {
	tidy();
}

string Viewport::getTag() const noexcept {
	return std::format(R"(Viewport <{:#x}>)", reinterpret_cast<u64>(this));
}

bool Viewport::shouldTick() const noexcept {
	return true;
}

void Viewport::tick() {
	Widget::tick();

	/**/

	if (_currentImageSignal != nullptr) {
		::hg::breakpoint();
	}

	/**/

	smr<gfx::Texture> image {};
	vk::Semaphore imageSignal {};
	Vector<vk::Semaphore> imageWaits {};

	const auto result { _swapChain->consumeNext(image, imageSignal, imageWaits) };
	if (not result) {
		IM_CORE_ERROR("Skipping viewport draw due to missing next swapchain frame.");

		_currentSwapChainImage = nullptr;
		_currentImageSignal = nullptr;
		_currentImageWaits.clear();
		return;
	}

	/**/

	_currentSwapChainImage = std::move(image);
	_currentImageSignal = reinterpret_cast<_::VkSemaphore>(imageSignal.operator VkSemaphore());

	_currentImageWaits.clear();
	for (auto&& wait : imageWaits) {
		_currentImageWaits.emplace_back(reinterpret_cast<_::VkSemaphore>(wait.operator VkSemaphore()));
	}
}

void Viewport::tidy() {
	if (_swapChain) {
		_swapChain->destroy();
		_swapChain.reset();
	}

	/**/

	_viewListen.clear();
	_viewListen.shrink_to_fit();
}

smr<engine::gfx::Swapchain> Viewport::getSwapchain() const noexcept {
	return clone(_swapChain);
}

bool Viewport::hasMountedRenderTarget() const noexcept {
	return _renderTarget != nullptr;
}

const non_owning_rptr<CameraActor> Viewport::getCameraActor() const noexcept {
	return _cameraActor;
}

void Viewport::setViewportTarget(StringView renderer_, Universe universe_, ptr<CameraActor> camera_) {
	_renderer = std::move(renderer_);
	_cameraUniverse = std::move(universe_);
	_cameraActor = camera_;
}

math::uivec2 Viewport::actualViewExtent() const noexcept {
	if (_viewSize.allZero()) {
		return math::uivec2 {
			static_cast<u32>(_layoutState.layoutSize.x),
			static_cast<u32>(_layoutState.layoutSize.y)
		};
	}

	return _viewSize;
}

bool Viewport::viewHasChanged() const noexcept {
	if (!_swapChain) {
		return true;
	}

	const auto size = math::compMax<u32>(actualViewExtent(), math::uivec2 { 1uL });
	return _swapChain->extent() != size;
}

smr<engine::gfx::VkSwapchain> Viewport::buildNextView(cref<math::uivec2> extent_) const {

	auto next = make_smr<gfx::VkSwapchain>();

	next->setExtent(extent_);
	next->setFormat(gfx::TextureFormat::eB8G8R8A8Unorm);
	next->setDesiredImages(2uL);

	const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
	next->setup(device);

	return next;
}

void Viewport::remountRenderTarget() {

	// Warning: Temporary Internal Linking

	const auto gfx = Engine::getEngine()->getGraphics();
	auto* const manager = gfx->getSceneManager();

	const auto renderer = gfx->getRenderer(_renderer, std::nothrow);
	const auto* const coreUniverse = static_cast<::hg::engine::core::Universe*>(_cameraUniverse.unwrap().get());
	const auto coreScene = coreUniverse->getScene();

	auto view = smr<const hg::engine::gfx::scene::SceneView> { nullptr };

	const auto renderSystem = coreScene->getSceneSystem<render::RenderSceneSystem>();
	renderSystem->getRegistry().forEach<render::CameraModel>(
		[owner = _cameraActor->getCameraComponent(), &view](const auto& model_) {
			if (model_.owner() == owner) {
				view = model_.getSceneView();
			}
		}
	);

	/**/

	if (view.empty()) {
		return;
	}

	/**/

	auto renderTarget = make_smr<gfx::RenderTarget>(gfx->getCurrentDevice(), renderer.get());
	[[maybe_unused]] const auto targetTransition = renderTarget->transitionToTarget(clone(_swapChain), nullptr);
	[[maybe_unused]] const auto sceneViewTransition = renderTarget->transitionToSceneView(std::move(view));

	renderTarget->setup();
	renderTarget->setActive(true);

	/**/

	_renderTarget = renderTarget.get();
	manager->addPrimaryTarget(std::move(renderTarget));
}

void Viewport::rebuildView() {

	// Error: Will cause nullpointer exceptions
	// TODO: Introduce Empty/Null-Swapchain or deal with zero-dimension case
	auto extent { actualViewExtent() };
	if (extent.anyZero()) {
		IM_CORE_WARNF(
			"Tried to rebuild Swapchain for Viewport (`{:#x}`) with zero dimension.",
			reinterpret_cast<volatile ptrdiff_t>(this)
		);
		extent = math::compMax<u32>(extent, math::uivec2 { 1uL });
	}

	auto next = buildNextView(extent);

	/**/

	if (_cameraActor) {
		const auto cc { _cameraActor->getCameraComponent() };
		cc->setAspectRatio(
			static_cast<float>(extent.x) / static_cast<float>(extent.y)
		);
	}

	/**/

	handleViewListener(_swapChain, next);
	_swapChain = std::move(next);
}

void Viewport::handleViewListener(cref<smr<gfx::VkSwapchain>> prev_, cref<smr<gfx::VkSwapchain>> next_) {
	for (const auto& fnc : _viewListen) {
		fnc(clone(prev_), clone(next_));
	}
}

void Viewport::resizeView(cref<math::uivec2> extent_) {
	_viewSize = extent_;
}

void Viewport::addResizeListener(
	mref<std::function<void(mref<smr<gfx::VkSwapchain>>, mref<smr<gfx::VkSwapchain>>)>> fnc_
) {
	_viewListen.push_back(std::move(fnc_));
}

void Viewport::removeResizeListener() {
	::hg::todo_panic();
}

const ptr<const NullChildren> Viewport::children() const {
	return get_null_children();
}

void Viewport::render(const ptr<ReflowCommandBuffer> cmd_) {

	if (_layoutState.layoutSize.anyZero()) {
		return;
	}

	if (viewHasChanged()) {
		rebuildView();

		/*
		if (gfx->_renderTarget->ready()) {
		    gfx->_renderTarget->rebuildPasses(_swapChain.get());
		} else {
		    gfx->_renderTarget->use(_swapChain.get());
		    gfx->_renderTarget->buildPasses(_camera.get());
		}
		 */
	}

	if (_renderTarget == nullptr) {
		remountRenderTarget();
	}

	/**/

	const auto offset = _layoutState.layoutOffset;
	const auto size = _layoutState.layoutSize;

	if (_currentSwapChainImage.empty()) {
		return;
	}

	/**/

	_currentSwapChainImage->buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	cmd_->drawImageAsync(
		math::vec2 { offset.x, offset.y },
		_uvs[0],
		math::vec2 { offset.x + size.x, offset.y },
		_uvs[1],
		math::vec2 { offset.x + size.x, offset.y + size.y },
		_uvs[2],
		math::vec2 { offset.x, offset.y + size.y },
		_uvs[3],
		_currentSwapChainImage.get(),
		_currentImageWaits.empty() ? nullptr : _currentImageWaits.back(),
		_currentImageSignal,
		{
			ReflowAlphaMode::eNone,
			engine::color { 255.F, 255.F, 255.F, 255.F }
		}
	);

	// Error: This will break any asynchronous processing
	_currentImageWaits.clear();
	_currentImageSignal = nullptr;
}

PrefetchSizing Viewport::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	::hg::todo_panic();
}

PassPrefetchSizing Viewport::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	::hg::todo_panic();
}

void Viewport::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	::hg::todo_panic();
}

void Viewport::applyLayout(ref<ReflowState> state_) {}

math::fvec2 Viewport::getShrinkFactor() const noexcept {
	::hg::todo_panic();
}

math::fvec2 Viewport::getGrowFactor() const noexcept {
	::hg::todo_panic();
}

EventResponse Viewport::invokeOnFocus(cref<FocusEvent> event_) {
	_state.set(WidgetStateFlagBits::eFocus);
	markAsPending();
	return EventResponse::eConsumed;
}

EventResponse Viewport::invokeOnBlur(cref<FocusEvent> event_) {
	_state.unset(WidgetStateFlagBits::eFocus);
	markAsPending();
	return EventResponse::eConsumed;
}

EventResponse Viewport::invokeOnKeyDown(ref<const KeyboardEvent> event_) {
	// Early exit on ESC Key to drop focus and ctrl
	if (event_._key == '\x1B') {
		_state.unset(WidgetStateFlagBits::eFocus);
		markAsPending();
		return EventResponse::eConsumed;
	}

	if (not _cameraActor) {
		return EventResponse::eHandled;
	}

	const auto isShift { (event_._modifier & 0x3) != 0x0 };
	const float factor { 0.1F };

	auto response { EventResponse::eHandled };
	switch (event_._key) {
		case 'a': {
			response = EventResponse::eConsumed;

			ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };

			auto rlwd = rtf.rotator().into()._left();

			rtf.setLocation(
				math::Location(
					rtf.location().into() + rlwd.normalize() * factor
				)
			);
			break;
		}
		case 'd': {
			response = EventResponse::eConsumed;

			ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };

			auto rrwd = -rtf.rotator().into()._left();

			rtf.setLocation(
				math::Location(
					rtf.location().into() + rrwd.normalize() * factor
				)
			);
			break;
		}
		case 's': {
			response = EventResponse::eConsumed;

			ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };
			cref<Transform> ctf = _cameraActor->getCameraComponent()->getLocalTransform();

			const auto rotator = math::Rotator::combine(rtf.rotator(), ctf.rotator());
			auto rbwd = -rotator.into().__forward();
			rbwd.x *= -1.F;

			rtf.setLocation(
				math::Location(
					rtf.location().into() + rbwd.normalize() * factor
				)
			);
			break;
		}
		case 'w': {
			response = EventResponse::eConsumed;

			ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };
			cref<Transform> ctf = _cameraActor->getCameraComponent()->getLocalTransform();

			const auto rotator = math::Rotator::combine(rtf.rotator(), ctf.rotator());
			auto rfwd = rotator.into().__forward();
			rfwd.x *= -1.F;

			rtf.setLocation(
				math::Location(
					rtf.location().into() + rfwd.normalize() * factor
				)
			);
			break;
		}
		case 'q': {
			response = EventResponse::eConsumed;

			ref<Transform> tf { _cameraActor->getRootComponent()->getLocalTransform() };

			tf.setLocation(
				math::Location(
					tf.location().into() + math::vec3_down * factor
				)
			);
			break;
		}

		case 'c': {
			response = EventResponse::eConsumed;

			ref<Transform> tf { _cameraActor->getRootComponent()->getLocalTransform() };

			tf.setLocation(
				math::Location(
					tf.location().into() + math::vec3_up * factor
				)
			);
			break;
		}
		default: {}
	}

	return response;
}

EventResponse Viewport::invokeOnKeyUp(ref<const KeyboardEvent> event_) {
	return Widget::invokeOnKeyUp(event_);
}

EventResponse Viewport::invokeOnMouseEnter(ref<const MouseMoveEvent> event_) {
	return Widget::invokeOnMouseEnter(event_);
}

EventResponse Viewport::invokeOnMouseMove(ref<const MouseMoveEvent> event_) {
	const auto isShift { (event_._modifier & 0x3) != 0x0 };
	const auto rbtn { (event_._button & 0x4) != 0x0 };

	if (!rbtn || not _cameraActor) {
		return EventResponse::eConsumed;
	}

	constexpr float deltaTime { (1000.F / 60.F) / 1000.F };
	constexpr float speed { 4.F };

	math::vec2 dxdy {
		math::vec2 {
			static_cast<float>(event_._delta.x),
			static_cast<float>(event_._delta.y)
		}
		* speed
		* deltaTime
	};

	dxdy.x = glm::radians(dxdy.x);
	dxdy.y = glm::radians(dxdy.y);

	ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };
	ref<Transform> ctf { _cameraActor->getCameraComponent()->getLocalTransform() };

	rtf.rotator() += math::Rotator { 0.F, dxdy.x, 0.F };
	ctf.rotator() += math::Rotator { dxdy.y, 0.F, 0.F };

	return EventResponse::eConsumed;
}

EventResponse Viewport::invokeOnMouseLeave(ref<const MouseMoveEvent> event_) {
	return Widget::invokeOnMouseLeave(event_);
}
