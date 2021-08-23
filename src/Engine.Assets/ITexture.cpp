#include "ITexture.hpp"

using namespace asset::texture;

const std::string ITexture::getFile() const {
	std::string path = FileResource::getRootDir();
	path += "\\assets";
	path += "\\texture";

	return (path + "\\" + clockwork::__internal::utils::StringUtils::toLowerCase(_file) + "." +
		FileResource::getFileEnding(_type));
}
