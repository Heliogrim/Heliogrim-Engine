#include "Viewport.hpp"

#include "Engine.Common/Math/Convertion.hpp"
#include "Engine.GFX/Swapchain/VkSwapchain.hpp"

#if TRUE
#include <Engine.Logging/Logger.hpp>
#include <Engine.GFX/Graphics.hpp>
#include "Engine.GFX/RenderTarget.hpp"
#include <Engine.Common/Math/Coordinates.hpp>
#include <Heliogrim/Actors/CameraActor.hpp>
#include <Engine.Core/Engine.hpp>
#endif

#include <Engine.Core/World.hpp>
#include <Engine.GFX.Scene/RenderSceneManager.hpp>
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
	_cameraActor(nullptr),
	_renderer(),
	_renderTarget(nullptr),
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

void Viewport::setViewportTarget(StringView renderer_, World world_, ptr<CameraActor> camera_) {
	_renderer = std::move(renderer_);
	_cameraWorld = std::move(world_);
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

	const auto* const gfx = Engine::getEngine()->getGraphics();
	auto* const manager = gfx->getSceneManager();

	const auto renderer = gfx->getRenderer(_renderer, std::nothrow);
	const auto* const coreWorld = static_cast<::hg::engine::core::World*>(_cameraWorld.unwrap().get());
	const auto* const coreScene = coreWorld->getScene();

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
		auto* const cc { _cameraActor->getCameraComponent() };
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
	__debugbreak();
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

	smr<gfx::Texture> image {};
	vk::Semaphore imageSignal {};
	Vector<vk::Semaphore> imageWaits {};

	const auto result { _swapChain->consumeNext(image, imageSignal, imageWaits) };
	if (!result) {
		IM_CORE_ERROR("Skipping viewport draw due to missing next swapchain frame.");
	}

	if (result) {

		image->buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		cmd_->drawImageAsync(
			math::vec2 { offset.x, offset.y },
			_uvs[0],
			math::vec2 { offset.x + size.x, offset.y },
			_uvs[1],
			math::vec2 { offset.x + size.x, offset.y + size.y },
			_uvs[2],
			math::vec2 { offset.x, offset.y + size.y },
			_uvs[3],
			image.get(),
			reinterpret_cast<_::VkSemaphore>(imageWaits.empty() ? nullptr : imageWaits.back().operator VkSemaphore()),
			reinterpret_cast<_::VkSemaphore>(imageSignal.operator VkSemaphore()),
			{
				ReflowAlphaMode::eNone,
				engine::color { 255.F, 255.F, 255.F, 255.F }
			}
		);
	}
}

math::vec2 Viewport::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

	math::vec2 size = math::vec2 { 0.F };

	/**/

	if (attr.width->type == ReflowUnitType::eAbsolute) {
		size.x = attr.width->value;
	}
	if (attr.height->type == ReflowUnitType::eAbsolute) {
		size.y = attr.height->value;
	}

	/**/

	return layout::clampSizeAbs(attr, size);
}

math::vec2 Viewport::computeDesiredSize(cref<ReflowPassState> passState_) const {

	math::vec2 desired { getDesiredSize() };
	if (attr.width->type == ReflowUnitType::eRelative) {
		desired.x = passState_.referenceSize.x * attr.width->value;
	}
	if (attr.height->type == ReflowUnitType::eRelative) {
		desired.y = passState_.referenceSize.y * attr.height->value;
	}

	return layout::clampSize(
		attr,
		passState_.layoutSize,
		desired
	);
}

void Viewport::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {}

EventResponse Viewport::onFocus(cref<FocusEvent> event_) {
	_state.set(WidgetStateFlagBits::eFocus);
	markAsPending();
	return EventResponse::eConsumed;
}

EventResponse Viewport::onBlur(cref<FocusEvent> event_) {
	_state.unset(WidgetStateFlagBits::eFocus);
	markAsPending();
	return EventResponse::eConsumed;
}

EventResponse Viewport::onKeyDown(cref<KeyboardEvent> event_) {
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

			auto rlwd = rtf.rotator().quaternion()._left();

			rtf.setLocation(
				math::Location(
					rtf.location().fvec3() + rlwd.normalize() * factor
				)
			);
			break;
		}
		case 'd': {
			response = EventResponse::eConsumed;

			ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };

			auto rrwd = -rtf.rotator().quaternion()._left();

			rtf.setLocation(
				math::Location(
					rtf.location().fvec3() + rrwd.normalize() * factor
				)
			);
			break;
		}
		case 's': {
			response = EventResponse::eConsumed;

			ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };
			cref<Transform> ctf = _cameraActor->getCameraComponent()->getLocalTransform();

			const auto rotator = math::Rotator::combine(rtf.rotator(), ctf.rotator());
			auto rbwd = -rotator.quaternion().__forward();
			rbwd.x *= -1.F;

			rtf.setLocation(
				math::Location(
					rtf.location().fvec3() + rbwd.normalize() * factor
				)
			);
			break;
		}
		case 'w': {
			response = EventResponse::eConsumed;

			ref<Transform> rtf { _cameraActor->getRootComponent()->getLocalTransform() };
			cref<Transform> ctf = _cameraActor->getCameraComponent()->getLocalTransform();

			const auto rotator = math::Rotator::combine(rtf.rotator(), ctf.rotator());
			auto rfwd = rotator.quaternion().__forward();
			rfwd.x *= -1.F;

			rtf.setLocation(
				math::Location(
					rtf.location().fvec3() + rfwd.normalize() * factor
				)
			);
			break;
		}
		case 'q': {
			response = EventResponse::eConsumed;

			ref<Transform> tf { _cameraActor->getRootComponent()->getLocalTransform() };

			tf.setLocation(
				math::Location(
					tf.location().fvec3() + math::vec3_down * factor
				)
			);
			break;
		}

		case 'c': {
			response = EventResponse::eConsumed;

			ref<Transform> tf { _cameraActor->getRootComponent()->getLocalTransform() };

			tf.setLocation(
				math::Location(
					tf.location().fvec3() + math::vec3_up * factor
				)
			);
			break;
		}
		default: {}
	}

	return response;
}

EventResponse Viewport::onKeyUp(cref<KeyboardEvent> event_) {
	return Widget::onKeyUp(event_);
}

EventResponse Viewport::onMouseEnter(cref<MouseMoveEvent> event_) {
	return Widget::onMouseEnter(event_);
}

EventResponse Viewport::onMouseMove(cref<MouseMoveEvent> event_) {
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

EventResponse Viewport::onMouseLeave(cref<MouseMoveEvent> event_) {
	return Widget::onMouseLeave(event_);
}
