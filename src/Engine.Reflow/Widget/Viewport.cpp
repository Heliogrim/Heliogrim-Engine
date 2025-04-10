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
#include <Engine.Core/Timing.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.GFX.Scene/RenderSceneManager.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Render.Scene.Model/CameraModel.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Scene/SceneBase.hpp>

#include "../Algorithm/Flex.hpp"
#include "../Layout/Style.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Viewport::Viewport() :
	Widget(),
	Atom(
		{},
		{
			BoxLayoutAttributes {
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				0.F, 1.F,
				/**/
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				ReflowUnit { ReflowUnitType::eAuto, 0.F },
				0.F, 1.F,
				/**/
				Padding { 0.F },
				ReflowPlacement::eMiddleCenter
			}
		},
		{}
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

	if (_cameraUniverse.valid() && _cameraActor != nullptr) {
		updateCameraWithCtrl();
	}

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

void Viewport::updateCameraWithCtrl() {

	ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };
	ref<const Transform> ctf = _cameraActor->getCameraComponent()->getLocalTransform();
	const auto factor = (_cameraControls.boost ? 20.F : 5.F) * core::Timing::getDeltaTime();

	auto delta = math::fvec3 { 0.F };

	/**/

	if (_cameraControls.forward && not _cameraControls.backward) {

		const auto rotator = math::Rotator::combine(rtf.rotator(), ctf.rotator());
		auto rfwd = rotator.into().__forward();
		rfwd.x *= -1.F;

		delta += rfwd.normalize();
	}

	if (_cameraControls.left && not _cameraControls.right) {

		auto rlwd = -rtf.rotator().into()._left();
		delta += rlwd.normalize();
	}

	if (not _cameraControls.forward && _cameraControls.backward) {

		const auto rotator = math::Rotator::combine(rtf.rotator(), ctf.rotator());
		auto rbwd = -rotator.into().__forward();
		rbwd.x *= -1.F;

		delta += rbwd.normalize();
	}

	if (not _cameraControls.left && _cameraControls.right) {

		auto rrwd = rtf.rotator().into()._left();

		delta += rrwd.normalize();
	}

	if (_cameraControls.up && not _cameraControls.down) {
		delta += math::vec3_up;
	}

	if (not _cameraControls.up && _cameraControls.down) {
		delta += math::vec3_down;
	}

	/**/

	if (not delta.zero()) {
		rtf.setLocation(math::Location(rtf.location().into() + delta.normalize() * factor));
	}
}

math::uivec2 Viewport::actualViewExtent() const noexcept {
	if (_viewSize.allZero()) {
		const auto padding = getLayoutAttributes().valueOf<attr::BoxLayout::padding>();
		return math::uivec2 {
			static_cast<u32>(_layoutState.layoutSize.x - (padding.x + padding.z)),
			static_cast<u32>(_layoutState.layoutSize.y - (padding.y + padding.w))
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

	// Warning: We need the actual layout size, so hoisting into the tick function is not possible
	auto& mutSelf = const_cast<ref<this_type>>(*this);

	if (viewHasChanged()) {
		mutSelf.rebuildView();

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
		mutSelf.remountRenderTarget();
	}

	/**/

	const auto& box = getLayoutAttributes();
	const auto padding = box.valueOf<attr::BoxLayout::padding>();

	const auto offset = _layoutState.layoutOffset + math::fvec2 { padding.x, padding.y };
	const auto size = _layoutState.layoutSize - math::fvec2 { padding.x + padding.z, padding.y + padding.w };

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
	mutSelf._currentImageWaits.clear();
	mutSelf._currentImageSignal = nullptr;
}

PrefetchSizing Viewport::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {

	const auto& box = getLayoutAttributes();

	return algorithm::prefetch(
		axis_,
		{
			.mainAxis = ReflowAxis::eXAxis,
			.min = algorithm::unitAbsMin(box.valueOf<attr::BoxLayout::minWidth>(), box.valueOf<attr::BoxLayout::minHeight>()),
			.max = algorithm::unitAbsMax(box.valueOf<attr::BoxLayout::maxWidth>(), box.valueOf<attr::BoxLayout::maxHeight>()),
			.gapping = { 0.F, 0.F },
			.padding = box.valueOf<attr::BoxLayout::padding>()
		},
		get_null_children()
	);
}

PassPrefetchSizing Viewport::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {

	const auto& box = getLayoutAttributes();
	return {
		algorithm::nextUnit(
			box.valueOf<attr::BoxLayout::minWidth>(),
			box.valueOf<attr::BoxLayout::minHeight>(),
			passState_.referenceSize,
			passState_.prefetchMinSize
		),
		math::compMax(
			algorithm::nextUnit(
				box.valueOf<attr::BoxLayout::minWidth>(),
				box.valueOf<attr::BoxLayout::minHeight>(),
				passState_.referenceSize,
				passState_.prefetchSize
			),
			passState_.prefetchSize
		),
		algorithm::nextUnit(
			box.valueOf<attr::BoxLayout::maxWidth>(),
			box.valueOf<attr::BoxLayout::maxHeight>(),
			passState_.referenceSize,
			algorithm::unitAbsMax(
				box.valueOf<attr::BoxLayout::maxWidth>(),
				box.valueOf<attr::BoxLayout::maxHeight>()
			)
		)
	};
}

math::fvec2 Viewport::computeReferenceSize(ReflowAxis axis_) const {
	return math::vec2_zero;
}

void Viewport::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {}

void Viewport::applyLayout(ref<ReflowState> state_) {}

math::fvec2 Viewport::getShrinkFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthShrink>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightShrink>()
	};
}

math::fvec2 Viewport::getGrowFactor() const noexcept {
	return {
		getLayoutAttributes().valueOf<attr::BoxLayout::widthGrow>(),
		getLayoutAttributes().valueOf<attr::BoxLayout::heightGrow>()
	};
}

EventResponse Viewport::invokeOnFocus(cref<FocusEvent> event_) {
	_state.set(WidgetStateFlagBits::eFocus);
	markAsPending(false, true);
	return EventResponse::eConsumed;
}

EventResponse Viewport::invokeOnBlur(cref<FocusEvent> event_) {

	_cameraControls.forward = false;
	_cameraControls.left = false;
	_cameraControls.backward = false;
	_cameraControls.right = false;
	_cameraControls.up = false;
	_cameraControls.down = false;
	_cameraControls.boost = false;

	/**/

	_state.unset(WidgetStateFlagBits::eFocus);
	markAsPending(false, true);
	return EventResponse::eConsumed;
}

EventResponse Viewport::invokeOnKeyDown(ref<const KeyboardEvent> event_) {
	// Early exit on ESC Key to drop focus and ctrl
	if (event_._key == '\x1B') {
		_state.unset(WidgetStateFlagBits::eFocus);
		markAsPending(false, true);
		return EventResponse::eConsumed;
	}

	if (not _cameraActor) {
		return EventResponse::eHandled;
	}

	auto response { EventResponse::eHandled };
	switch (event_._key) {
		case 'a': {
			_cameraControls.left = true;
			response = EventResponse::eConsumed;
			break;
		}
		case 'd': {
			_cameraControls.right = true;
			response = EventResponse::eConsumed;
			break;
		}
		case 's': {
			_cameraControls.backward = true;
			response = EventResponse::eConsumed;
			break;
		}
		case 'w': {
			_cameraControls.forward = true;
			response = EventResponse::eConsumed;
			break;
		}
		case 'q': {
			_cameraControls.down = true;
			response = EventResponse::eConsumed;
			break;
		}
		case 'c': {
			_cameraControls.up = true;
			response = EventResponse::eConsumed;
			break;
		}
		default: {}
	}

	if ((event_._modifier & 0x3) != 0x0) {
		_cameraControls.boost = true;
	}

	return response;
}

EventResponse Viewport::invokeOnKeyUp(ref<const KeyboardEvent> event_) {

	// Early exit on ESC Key to drop focus and ctrl
	if (event_._key == '\x1B') {
		_state.unset(WidgetStateFlagBits::eFocus);
		markAsPending(false, true);
		return EventResponse::eConsumed;
	}

	if (not _cameraActor) {
		return EventResponse::eHandled;
	}

	auto response { EventResponse::eHandled };
	switch (event_._key) {
		case 'a': {
			_cameraControls.left = false;
			response = EventResponse::eConsumed;
			break;
		}
		case 'd': {
			_cameraControls.right = false;
			response = EventResponse::eConsumed;
			break;
		}
		case 's': {
			_cameraControls.backward = false;
			response = EventResponse::eConsumed;
			break;
		}
		case 'w': {
			_cameraControls.forward = false;
			response = EventResponse::eConsumed;
			break;
		}
		case 'q': {
			_cameraControls.down = false;
			response = EventResponse::eConsumed;
			break;
		}
		case 'c': {
			_cameraControls.up = false;
			response = EventResponse::eConsumed;
			break;
		}
		default: {}
	}

	if ((event_._modifier & 0x3) == 0x0) {
		_cameraControls.boost = false;
	}

	return response;
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
	ctf.rotator() += math::Rotator { -dxdy.y, 0.F, 0.F };

	return EventResponse::eConsumed;
}

EventResponse Viewport::invokeOnMouseLeave(ref<const MouseMoveEvent> event_) {
	return Widget::invokeOnMouseLeave(event_);
}
