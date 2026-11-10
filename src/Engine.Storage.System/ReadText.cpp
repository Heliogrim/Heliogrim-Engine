#include "ReadText.hpp"

#include <filesystem>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>

#include "StorageSystem.hpp"

using namespace hg;
using namespace hg::engine::storage;

Opt<String> engine::storage::readTextViaStorage(
	ref<StorageSystem> system_,
	ref<const UrlScheme> scheme_,
	ref<const fs::Path> absolutePath_
) {
	// Note: Check first, so a missing candidate does not create a storage for a nonexistent file.
	const auto& native = static_cast<cref<std::filesystem::path>>(absolutePath_);
	// Note: Only absolute paths to regular files resolve in storage repositories.
	if (not native.is_absolute() || not std::filesystem::is_regular_file(native)) {
		return None;
	}

	auto url = FileUrl { scheme_, fs::Path { absolutePath_ } };

	// Note: `requestStorage` returns the known storage, so no separate `hasStorage` branch is needed.
	auto requested = system_.requestStorage({ .fileUrl = ::hg::move(url) });
	// Note: An unknown scheme yields a null storage instead of an error.
	if (not requested.has_value() || requested.value() == nullptr) {
		return None;
	}

	auto store = ::hg::move(requested).value().into<IStorage>();

	auto text = Opt<String> {};
	const auto queried = system_.query(
		::hg::move(store),
		[&text](_In_ ref<const resource::Blob> blob_) {

			const auto size = blob_.size();
			if (size < 0) {
				return;
			}

			auto bytes = String(static_cast<size_t>(size), '\0');
			const auto got = blob_.read(0, std::span { reinterpret_cast<ptr<_::byte>>(bytes.data()), bytes.size() });
			bytes.resize(got.size());
			text = Some(::hg::move(bytes));
		}
	);

	return queried.has_value() ? ::hg::move(text) : Opt<String> { None };
}
