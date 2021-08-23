#include "IAsset.hpp"
#include <Engine.Utils/StringUtils.hpp>

using namespace asset::subject;

std::string IAsset::getAssetPath() const
{
	std::string path = FileResource::getRootDir();
	path += "\\assets";

	if (isStatic())
		path += "\\static";
	else
		path += "\\entity";

	// static path
	if (isBuilding())
		path += "\\building";
	if (isPlant())
		path += "\\plant";

	// entity path
	if (isLiving())
		path += "\\living";

	return (path + "\\" + clockwork::__internal::utils::StringUtils::toLowerCase(_classname) + "\\");
}


IAsset::identifier IAsset::getAssetId() const
{
	return _id;
}

std::string IAsset::getName() const
{
	return _classname;
}

std::string IAsset::getModelFile() const
{
	return (getAssetPath() + clockwork::__internal::utils::StringUtils::toLowerCase(_classname) + "." + FileResource::getFileEnding(_modelType));
}

ModelData IAsset::getModelData() const
{
	return _modelData;
}

std::vector<std::string> IAsset::getMaterials() const
{
	return std::vector<std::string>(_materials);
}