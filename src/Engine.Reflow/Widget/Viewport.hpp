#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflow.Uikit/Atom/Atom.hpp>
#include <Heliogrim/Universe/Universe.hpp>

#include "Widget.hpp"
#include "../Children.hpp"

namespace hg {
	class CameraActor;
}

namespace hg::engine::gfx {
	class Swapchain;
	class VkSwapchain;
	class RenderTarget;
}

namespace hg::engine::reflow {
	class Viewport :
		public Widget,
		public uikit::Atom<void, BoxLayoutAttributes, void> {
	public:
		using this_type = Viewport;

	public:
		Viewport();

		~Viewport() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		bool shouldTick() const noexcept override;

		void tick() override;

	private:
		void tidy();

	private:
		smr<gfx::VkSwapchain> _swapChain;
		nmpt<gfx::RenderTarget> _renderTarget;

		smr<gfx::Texture> _currentSwapChainImage;
		::hg::_::VkSemaphore _currentImageSignal;
		Vector<::hg::_::VkSemaphore> _currentImageWaits;

	protected:
		StringView _renderer;
		ptr<CameraActor> _cameraActor;
		Universe _cameraUniverse;

	public:
		[[nodiscard]] smr<gfx::Swapchain> getSwapchain() const noexcept;

		[[nodiscard]] bool hasMountedRenderTarget() const noexcept;

		[[nodiscard]] const non_owning_rptr<CameraActor> getCameraActor() const noexcept;

		void setViewportTarget(StringView renderer_, Universe universe_, ptr<CameraActor> camera_);

	private:
		std::array<math::vec2, 4> _uvs;

		math::uivec2 _viewSize;
		Vector<std::function<void(mref<smr<gfx::VkSwapchain>>, mref<smr<gfx::VkSwapchain>>)>> _viewListen;

	private:
		[[nodiscard]] math::uivec2 actualViewExtent() const noexcept;

		[[nodiscard]] bool viewHasChanged() const noexcept;

		void handleViewListener(cref<smr<gfx::VkSwapchain>> prev_, cref<smr<gfx::VkSwapchain>> next_);

		[[nodiscard]] smr<gfx::VkSwapchain> buildNextView(cref<math::uivec2> extent_) const;

		void remountRenderTarget();

	public:
		void rebuildView();

		void resizeView(cref<math::uivec2> extent_);

		void addResizeListener(
			mref<std::function<void(mref<smr<gfx::VkSwapchain>>, mref<smr<gfx::VkSwapchain>>)>> fnc_
		);

		void removeResizeListener();

	public:
		const ptr<const NullChildren> children() const override;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		math::fvec2 computeReferenceSize(ReflowAxis axis_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		[[nodiscard]] math::fvec2 getShrinkFactor() const noexcept override;

		[[nodiscard]] math::fvec2 getGrowFactor() const noexcept override;

	public:
		EventResponse invokeOnFocus(cref<FocusEvent> event_) override;

		EventResponse invokeOnBlur(cref<FocusEvent> event_) override;

		EventResponse invokeOnKeyDown(ref<const KeyboardEvent> event_) override;

		EventResponse invokeOnKeyUp(ref<const KeyboardEvent> event_) override;

		EventResponse invokeOnMouseEnter(ref<const MouseMoveEvent> event_) override;

		EventResponse invokeOnMouseMove(ref<const MouseMoveEvent> event_) override;

		EventResponse invokeOnMouseLeave(ref<const MouseMoveEvent> event_) override;
	};
}
