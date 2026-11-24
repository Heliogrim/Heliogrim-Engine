#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	class Widget;

	// Precondition: the widget outlives the batch; the batch holds a plain reference and marks it on destruction.
	/**
	 * Defers the pending marks of attribute writes on one widget until the outermost batch closes.
	 */
	class AttributeBatch final {
	public:
		explicit AttributeBatch(ref<Widget> widget_) noexcept;

		AttributeBatch(ref<const AttributeBatch>) = delete;

		AttributeBatch(mref<AttributeBatch>) = delete;

		~AttributeBatch();

	public:
		ref<AttributeBatch> operator=(ref<const AttributeBatch>) = delete;

		ref<AttributeBatch> operator=(mref<AttributeBatch>) = delete;

	private:
		ref<Widget> _widget;
	};
}
