#pragma once

#include "../IPlant.hpp"

namespace asset::subject
{
	class LeaveTree :
		public IPlant
	{
	public:
		DEFAULT_CONSTRUCTOR_5(LeaveTree, 503, FileResource::DataType::eFbx, std::vector<std::string>({ "Bark__503__", "Leaves__503__" }), ModelData(2, 4))
	};
}