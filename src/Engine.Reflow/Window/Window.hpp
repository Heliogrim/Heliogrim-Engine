#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Platform/NativeWindow.hpp>

#include "Layer.hpp"
#include "WindowType.hpp"
#include "../Children.hpp"
#include "../FocusPath.hpp"
#include "../Widget/Widget.hpp"
#include "../Widget/__fwd.hpp"

namespace hg::engine::reflow {
	class EventDispatcher;
}

namespace hg::engine::reflow {
	class Window :
		public Widget {
	public:
		friend class ::hg::engine::reflow::EventDispatcher;

	public:
		using this_type = Window;

	public:
		Window();

		~Window() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		void tidy();

	private:
		ptr<platform::NativeWindow> _nativeWindow;

	private:
		WindowType _type;

		math::vec2 _screenPos;
		math::vec2 _clientSize;

		bool _resizable;
		bool _closeable;

		string _title;

	public:
		void setClientSize(cref<math::vec2> nextClientSize_);

		math::vec2 getClientSize() const noexcept;

	private:
		FixedChildren<2> _children;

	public:
		void setTitleBar(sptr<Widget> titleBar_);

		void setContent(sptr<Widget> content_);

	public:
		[[nodiscard]] sptr<Widget> root() const override;

		[[nodiscard]] const ptr<const Children> children() const override;

	private:
		Vector<uptr<Layer>> _layers;

	public:
		[[nodiscard]] nmpt<Layer> requestLayer(cref<sptr<Host>> host_);

		void dropLayer(nmpt<Host> host_);

		void dropLayer(nmpt<Layer> layer_);

	private:
		FocusPath _focus;

	public:
		[[nodiscard]] cref<FocusPath> getFocusPath() const noexcept;

		[[nodiscard]] sptr<Widget> getFocusTarget() const noexcept;

	public:
		void setDataWatcher(mref<Opt<ref<DataWatcher>>> watcher_);

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
	};
}
