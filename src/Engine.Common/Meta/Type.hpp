#pragma once

namespace hg {
	template <typename Type_>
	concept CompleteType = (sizeof(Type_) > 0);
}
