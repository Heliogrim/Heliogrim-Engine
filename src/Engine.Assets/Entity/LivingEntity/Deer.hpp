#pragma once

#include "..\ILivingEntity.hpp"

namespace asset::subject
{
	class Deer :
		public ILivingEntity
	{
	public:
		DEFAULT_CONSTRUCTOR(Deer, 401)
	};
}