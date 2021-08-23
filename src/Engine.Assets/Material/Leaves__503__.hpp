#pragma once

#include "../IMaterial.hpp"

namespace asset::material {
	class Leaves__503__ :
		public IMaterial {
	public:
		DEFAULT_CONSTRUCTOR(Leaves__503__, "Leaves", std::string(), clockwork::__internal::types::vec3(0.15F),
			clockwork::__internal::types::vec3(0.05F), clockwork::__internal::types::vec3(0.03F), 8.0F);
	};
}
