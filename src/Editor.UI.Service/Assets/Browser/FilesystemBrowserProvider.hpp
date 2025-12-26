#pragma once

#include <Engine.Common/Sal.hpp>

#include "AssetBrowserFilter.hpp"
#include "AssetBrowserProvider.hpp"

namespace hg::editor::ui::service {
	class FilesystemBrowserProvider :
		public AssetBrowserProvider {
	public:
		using this_type = FilesystemBrowserProvider;

	public:
		FilesystemBrowserProvider();

		~FilesystemBrowserProvider() override = default;

	private:
		fs::Url _root;

	private:
		string normalizeFsPath(cref<std::filesystem::path> unnormalized_) const;

		std::filesystem::path expandPath(mref<std::filesystem::path> normalized_) const;

		bool retrieveFs(
			cref<fs::Url> url_,
			const bool directories_,
			_Inout_ ref<Vector<AssetBrowserEntry>> entries_
		) const;

	public:
		[[nodiscard]] bool effects(ref<const AssetBrowserFilter> filter_) const override;

		bool fetchItems(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> entries_) const override;

		bool fetchDirectories(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> directories_) const override;
	};
}