#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

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
