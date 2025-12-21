#pragma once

#include <Engine.Filesystem/Path.hpp>
#include <Engine.Storage.Registry/Url/Scheme.hpp>

namespace hg::engine::res {
	struct ImportDestination {
		fs::Path virtualBasePath;
		storage::UrlScheme virtualRemapScheme;
	};
}