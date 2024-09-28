#include "PlatformResizeEvent.hpp"

using namespace hg::engine::platform;
using namespace hg;

PlatformResizeEvent::PlatformResizeEvent(
	const non_owning_rptr<NativeWindow> window_,
	cref<math::ivec2> nextSize_
) :
	StatelessEvent(),
	_window(window_),
	_nextSize(nextSize_) {}

const non_owning_rptr<NativeWindow> PlatformResizeEvent::getWindow() const noexcept {
	return _window;
}

cref<math::ivec2> PlatformResizeEvent::getNextSize() const noexcept {
	return _nextSize;
}
