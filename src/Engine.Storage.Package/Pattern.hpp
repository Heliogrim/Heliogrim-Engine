#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>

namespace hg::engine::storage::package {
	[[nodiscard]] bool isPackageBlob(_In_ ref<const resource::Blob> blob_);

	[[nodiscard]] bool isPackageFileUrl(_In_ cref<storage::FileUrl> fileUrl_);

	[[nodiscard]] bool isPackageFile(_In_ cref<storage::FileUrl> fileUrl_, _In_ ref<const resource::Blob> blob_);
}
