#pragma once

#include <Engine.Common/Variant.hpp>

#include "MemoryRequestOptions.hpp"
#include "PackageRequestOptions.hpp"

namespace hg::engine::storage {
	/**/

	struct RequestArchiveStorageOptions {
		Variant<Arci<system::MemoryStorage>, Arci<system::PackageStorage>> storage;
	};

	using request_archive_error = Variant<std::invalid_argument>;
	using request_archive_located_error = Variant<std::invalid_argument, std::runtime_error>;

	/**/

	struct RequestArchiveMemoryOptions {
		RequestMemoryOptions memory;
	};

	using request_archive_memory_error = Variant<std::bad_alloc>;
	using request_archive_memory_located_error = Variant<std::bad_alloc, std::runtime_error>;

	/**/

	struct RequestArchivePackageStorageOptions {
		RequestPackageStorageOptions package;
	};

	struct RequestArchivePackageUrlOptions {
		RequestPackageUrlOptions package;
	};

	using request_archive_storage_error = Variant<
		std::invalid_argument, std::system_error, std::out_of_range
	>;
	using request_archive_storage_located_error = Variant<
		std::invalid_argument, std::system_error, std::out_of_range, std::runtime_error
	>;
}
