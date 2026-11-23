#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Storage.Registry;
#endif

namespace hg::engine::storage {
	/**/

	struct RequestPackageStorageOptions {
		Variant<Arci<system::LocalFileStorage>, Arci<system::MemoryStorage>> storage;
	};

	using request_package_error = Variant<std::invalid_argument, std::out_of_range>;
	using request_package_located_error = Variant<std::invalid_argument, std::out_of_range, std::runtime_error>;

	/**/

	struct RequestPackageUrlOptions {
		FileUrl fileUrl;
	};

	using request_package_url_error = Variant<std::invalid_argument, std::system_error>;
	using request_package_url_located_error = Variant<std::invalid_argument, std::system_error, std::runtime_error>;
}
