#include "RfclUrl.hpp"

#include <format>

using namespace hg::engine::storage;
using namespace hg;

String RfclUrl::encode() const {
	return std::format(R"({}:{})", _scheme, _schemeSpecificData);
}
