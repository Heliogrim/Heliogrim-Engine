#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/UniquePointer.hpp>
#include <Engine.Filesystem/Url.hpp>

#include "../../UiService.hpp"

namespace hg::editor::ui::service {
	struct AssetBrowserEntry;
	class AssetBrowserProvider;
}

namespace hg::editor::ui::service {
	class AssetBrowserService final :
		public UiService {
	public:
		using this_type = AssetBrowserService;

	public:
		AssetBrowserService();

		~AssetBrowserService() override;

	private:
		Vector<UniquePtr<AssetBrowserProvider>> _providers;

	public:
		void addProvider(mref<UniquePtr<AssetBrowserProvider>> provider_);

	private:
		void fetchProviderItems(ref<const fs::Url> url_, _Inout_ ref<Vector<AssetBrowserEntry>> items_) const;

		void fetchProviderDirectories(ref<const fs::Url> url_, _Inout_ ref<Vector<AssetBrowserEntry>> directories_) const;

	public:
		void fetchAll(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) const;

		void fetchItems(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> items_) const;

		void fetchDirectories(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> directories_) const;
	};
}
