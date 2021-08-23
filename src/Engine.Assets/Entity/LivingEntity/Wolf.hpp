#pragma once

#include "../ILivingEntity.hpp"

namespace asset::subject
{
	class Wolf :
		public ILivingEntity
	{
	public:
		DEFAULT_CONSTRUCTOR(Wolf, 402)
	};
}