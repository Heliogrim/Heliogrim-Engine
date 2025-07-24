#include "ArchiveUrl.hpp"

#include <format>
#include <Engine.Common/GuidFormat.hpp>

using namespace hg::engine::storage;
using namespace hg;

String ArchiveUrl::encode() const {
	return std::format(R"({}://{})", static_cast<StringView>(ArchiveScheme), encodeGuid4228(static_cast<Guid>(_guid)));
}
