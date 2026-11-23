#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets;
import Engine.Assets.System;
import Engine.Assets.Type;
import Engine.Common;
#endif

namespace hg::editor::ui {
	class AssetBrowserHelper final {
	public:
		using this_type = AssetBrowserHelper;

	public:
		[[nodiscard]] static nmpt<this_type> get();

		static nmpt<this_type> make();

		static void destroy();

	protected:
		/**
		 * Singleton Instance
		 */
		static uptr<AssetBrowserHelper> _instance;

		AssetBrowserHelper();

	public:
		~AssetBrowserHelper() noexcept;

	protected:
		void setup();

	private:
		nmpt<engine::assets::IAssetRegistry> _assetRegistry;

	private:
		ptr<engine::assets::TextureAsset> _defaultTypeIcon;
		DenseMap<AssetTypeId, ptr<engine::assets::TextureAsset>> _typeIconMap;

	private:
		ptr<engine::assets::TextureAsset> _directoryIcon;
		Vector<std::pair<string, ptr<engine::assets::TextureAsset>>> _directoryIcons;

	public:
		[[nodiscard]] ptr<engine::assets::TextureAsset> getItemIconByAssetType(
			cref<AssetTypeId> typeId_
		) const noexcept;

		[[nodiscard]] ptr<engine::assets::TextureAsset> getItemIconForDirectory(cref<string_view> name_) const noexcept;

		[[nodiscard]] string getAssetTypeName(cref<AssetTypeId> typeId_) const noexcept;
	};
}
