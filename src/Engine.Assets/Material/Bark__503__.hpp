#pragma once

#include "../IMaterial.hpp"

namespace asset::material
{
	class Bark__503__ :
		public IMaterial
	{
	public:
		DEFAULT_CONSTRUCTOR(Bark__503__, "Bark", std::string(), clockwork::__internal::types::vec3(0.15F), clockwork::__internal::types::vec3(0.02F), clockwork::__internal::types::vec3(0.15F), 8.0F);
	};
}