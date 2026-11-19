#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::storage::package {
	[[nodiscard]] bool isPackageBlob(_In_ ref<const resource::Blob> blob_);

	[[nodiscard]] bool isPackageFileUrl(_In_ cref<storage::FileUrl> fileUrl_);

	[[nodiscard]] bool isPackageFile(_In_ cref<storage::FileUrl> fileUrl_, _In_ ref<const resource::Blob> blob_);
}
