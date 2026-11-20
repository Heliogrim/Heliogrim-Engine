#pragma once

#include <Engine.Storage.Registry/Url/FileUrl.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::storage {
	struct RequestFileUrlOptions {
		FileUrl fileUrl;
	};

	using request_file_error = Variant<std::invalid_argument, std::system_error>;
}
