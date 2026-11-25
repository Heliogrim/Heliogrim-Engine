#pragma once

#include "../Atom/Paint.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflow;
#endif

namespace hg::engine::reflow::uikit {
	/**
	 * A paint which takes its style from a component style and its state layers, on top of an instance rule.
	 */
	class Surface :
		public Paint {
	public:
		Surface();

		~Surface() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		String component;
		theming::StyleRule<PaintStyleAttributes> instance;

	public:
		void onStyleStatesChanged() override;

		void cascadeContextChange(bool invalidate_) override;

	private:
		void restyle();
	};
}
