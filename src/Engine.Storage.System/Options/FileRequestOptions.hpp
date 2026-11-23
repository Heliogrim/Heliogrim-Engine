#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Storage.Registry;
#endif

namespace hg::engine::storage {
	struct RequestFileUrlOptions {
		FileUrl fileUrl;
	};

	using request_file_error = Variant<std::invalid_argument, std::system_error>;
}
