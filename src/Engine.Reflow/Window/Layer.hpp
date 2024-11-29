#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>

#include "__fwd.hpp"

namespace hg::engine::reflow {
	class Host;
}

namespace hg::engine::reflow {
	class Layer {
	public:
		using this_type = Layer;

	public:
		Layer(cref<sptr<Window>> window_, _In_ mref<wptr<Host>> host_);

		~Layer();

	private:
		sptr<Window> _window;

	public:
		[[nodiscard]] sptr<Window> getWindow() const noexcept;

	private:
		math::vec2 _screenPos;
		math::vec2 _screenSize;

	public:
		void setScreenPos(cref<math::vec2> screenPos_);

		[[nodiscard]] cref<math::vec2> getScreenPos() const noexcept;

		void overrideScreenSize(cref<math::vec2> screenSize_);

		[[nodiscard]] bool hasOverrideSize() const noexcept;

		[[nodiscard]] cref<math::vec2> getOverrideSize() const noexcept;

	private:
		wptr<Host> _host;

	public:
		[[nodiscard]] wptr<Host> getHost() const noexcept;
	};
}
