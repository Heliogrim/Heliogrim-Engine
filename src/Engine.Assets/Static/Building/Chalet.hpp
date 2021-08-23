#pragma once

#include "..\IBuilding.hpp"

namespace asset::subject
{
	class Chalet :
		public IBuilding
	{
	public:
		DEFAULT_CONSTRUCTOR(Chalet, 301)
	};
}