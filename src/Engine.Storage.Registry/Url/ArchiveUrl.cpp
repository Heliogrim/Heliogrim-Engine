#include "ArchiveUrl.hpp"

#include <Engine.Common/__macro.hpp>

import Engine.Common;

using namespace hg::engine::storage;
using namespace hg;

String ArchiveUrl::encode() const {
	return std::format(R"({}://{})", static_cast<StringView>(ArchiveScheme), encodeGuid4228(static_cast<Guid>(_guid)));
}
