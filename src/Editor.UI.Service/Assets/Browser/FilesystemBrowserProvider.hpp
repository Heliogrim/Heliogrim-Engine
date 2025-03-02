#pragma once

#include <Engine.Common/Sal.hpp>

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
		bool effects(ref<const fs::Url> url_) const override;

		bool fetchItems(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) const override;

		bool fetchDirectories(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> directories_) const override;
	};
}
