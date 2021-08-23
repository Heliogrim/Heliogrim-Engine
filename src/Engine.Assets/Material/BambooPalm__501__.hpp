#pragma once

#include "../IMaterial.hpp"

namespace asset::material {
	class BambooPalm__501__ :
		public IMaterial {
	public:
		DEFAULT_CONSTRUCTOR(BambooPalm__501__, "Bamboo Palm UVW", std::string(),
			clockwork::__internal::types::vec3(0.15F), clockwork::__internal::types::vec3(0.05F),
			clockwork::__internal::types::vec3(0.03F), 8.0F);
	};
}
