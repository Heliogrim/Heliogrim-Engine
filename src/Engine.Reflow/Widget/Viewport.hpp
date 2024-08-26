#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Heliogrim/Universe.hpp>

#include "Widget.hpp"
#include "../Children.hpp"
#include "../ReflowUnit.hpp"

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
		public Widget {
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

	public:
		struct Attributes {
			Attribute<ReflowUnit> minWidth;
			Attribute<ReflowUnit> width;
			Attribute<ReflowUnit> maxWidth;

			Attribute<ReflowUnit> minHeight;
			Attribute<ReflowUnit> height;
			Attribute<ReflowUnit> maxHeight;
		} attr;

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
		math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

		math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

		void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

	public:
		EventResponse onFocus(cref<FocusEvent> event_) override;

		EventResponse onBlur(cref<FocusEvent> event_) override;

		EventResponse onKeyDown(cref<KeyboardEvent> event_) override;

		EventResponse onKeyUp(cref<KeyboardEvent> event_) override;

		EventResponse onMouseEnter(cref<MouseMoveEvent> event_) override;

		EventResponse onMouseMove(cref<MouseMoveEvent> event_) override;

		EventResponse onMouseLeave(cref<MouseMoveEvent> event_) override;
	};
}
