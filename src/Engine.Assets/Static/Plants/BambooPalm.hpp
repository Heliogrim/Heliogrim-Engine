#pragma once

#include "../IPlant.hpp"

namespace asset::subject
{
	class BambooPalm :
		public IPlant
	{
	public:
		DEFAULT_CONSTRUCTOR_5(BambooPalm, 501, FileResource::DataType::eObj, { "BambooPalm__501__" }, {})
	};
}