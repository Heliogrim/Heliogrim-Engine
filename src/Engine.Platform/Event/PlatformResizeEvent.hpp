#pragma once

#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::engine::platform {
	class NativeWindow;
}

namespace hg::engine::platform {
	class PlatformResizeEvent final :
		public StatelessEvent {
	public:
		using this_type = PlatformResizeEvent;

	public:
		inline static constexpr event_type_id typeId { "PlatformResizeEvent"_typeId };

	public:
		PlatformResizeEvent(const non_owning_rptr<NativeWindow> window_, cref<math::ivec2> nextSize_);

		PlatformResizeEvent(cref<this_type> other_) noexcept = default;

		PlatformResizeEvent(mref<this_type> other_) noexcept = default;

		constexpr ~PlatformResizeEvent() noexcept override = default;

	private:
		non_owning_rptr<NativeWindow> _window;
		math::ivec2 _nextSize;

	public:
		[[nodiscard]] const non_owning_rptr<NativeWindow> getWindow() const noexcept;

		[[nodiscard]] cref<math::ivec2> getNextSize() const noexcept;
	};
}
