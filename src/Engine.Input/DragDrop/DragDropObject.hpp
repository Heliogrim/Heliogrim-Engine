#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::input {
	class DragDropObject {
	public:
		using this_type = DragDropObject;

	protected:
		DragDropObject() = default;

	public:
		virtual ~DragDropObject() = default;

	public:
		virtual bool storeFiles(cref<Vector<string>> paths_) = 0;

		virtual bool storeText(cref<string> text_) = 0;
	};
}
