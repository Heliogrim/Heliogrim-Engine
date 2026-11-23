#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Filesystem;
import Engine.Storage.Registry;
#endif

namespace hg::engine::res {
	struct ImportDestination {
		fs::Path virtualBasePath;
		storage::UrlScheme virtualRemapScheme;
	};
}