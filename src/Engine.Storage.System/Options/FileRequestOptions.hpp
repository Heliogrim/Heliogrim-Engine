#pragma once

#include <stdexcept>
#include <system_error>
#include <Engine.Common/Variant.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>

namespace hg::engine::storage {
	struct RequestFileUrlOptions {
		FileUrl fileUrl;
	};

	using request_file_error = Variant<std::invalid_argument, std::system_error>;
}
