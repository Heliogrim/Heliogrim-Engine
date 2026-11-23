#pragma once

#include <Engine.Storage.Registry/Url/Scheme.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Filesystem;
#endif

namespace hg::engine::res {
	struct ImportDestination {
		fs::Path virtualBasePath;
		storage::UrlScheme virtualRemapScheme;
	};
}