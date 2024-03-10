#pragma once

#include <Engine.Common/Sal.hpp>

#include "AssetBrowserProvider.hpp"

namespace hg::editor::ui {
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
		string normalizeFsPath(mref<string> unnormalized_);

		string expandPath(mref<string> normalized_);

		bool retrieveFs(
			cref<fs::Url> url_,
			const bool directories_,
			_Inout_ ref<Vector<AssetBrowserEntry>> entries_
		);

	public:
		bool effects(cref<fs::Url> url_) override;

		bool retrieve(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) override;

		bool retrieveDirectories(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> directories_) override;
	};
}
