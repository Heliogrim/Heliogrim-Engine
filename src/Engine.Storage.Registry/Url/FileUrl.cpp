#include "FileUrl.hpp"

#include <Engine.Common/GuidFormat.hpp>

using namespace hg::engine::storage;
using namespace hg;

String FileUrl::encode() const {
	return std::format(R"({}://{})", _scheme, static_cast<String>(_path));
}
