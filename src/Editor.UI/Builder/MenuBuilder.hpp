#pragma once

#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Common/Memory/SharedPointer.hpp>

#include "../Widget/Menu.hpp"

namespace hg::editor::ui {
	class MenuBuilder;

	struct MenuBuilderSubItem {
		MenuBuilderSubItem(ref<const MenuBuilderSubItem>) = delete;

		MenuBuilderSubItem(mref<MenuBuilderSubItem>) = delete;

	public:
		StringView _parentId;
		StringView _itemId;
		ref<MenuBuilder> _builder;

	public:
		MenuBuilderSubItem setTitle(StringView title_) &&;

		MenuBuilderSubItem setIcon(mref<TypedAssetGuid<engine::assets::TextureAsset>> icon_) &&;

		[[nodiscard]] MenuBuilderSubItem addSubItem(StringView itemId_);
	};

	struct MenuBuilderItem {
		MenuBuilderItem(ref<const MenuBuilderItem>) = delete;

		MenuBuilderItem(mref<MenuBuilderItem>) = delete;

	public:
		StringView _itemId;
		ref<MenuBuilder> _builder;

	public:
		MenuBuilderItem setTitle(StringView title_) &&;

		MenuBuilderItem setIcon(mref<TypedAssetGuid<engine::assets::TextureAsset>> icon_) &&;

		[[nodiscard]] MenuBuilderSubItem addSubItem(StringView itemId_);
	};

	class MenuBuilder {
	public:
		[[nodiscard]] MenuBuilderItem addItem(StringView itemId_);

		[[nodiscard]] MenuBuilderSubItem addSubItem(StringView parentItemId_, StringView itemId_);

	public:
		[[nodiscard]] SharedPtr<Menu> construct() const;
	};
}
