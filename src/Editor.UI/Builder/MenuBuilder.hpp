#pragma once

#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Common/Collection/StableUnorderedMap.hpp>
#include <Engine.Common/Memory/SharedPointer.hpp>

#include "../Widget/Menu.hpp"
#include "../Widget/MenuItem.hpp"

namespace hg::editor::ui {
	class MenuBuilder;

	/**/

	struct MenuItemData {
		String title;
		TypedAssetGuid<engine::assets::TextureAsset> icon;
		std::function<void()> action;

		/**/

		StableUnorderedMap<String, MenuItemData> subItems;
	};

	/**/

	struct MenuBuilderSubItem {
		constexpr MenuBuilderSubItem(ref<decltype(MenuItemData::subItems)::value_type> self_, ref<MenuBuilder> builder_) noexcept :
			_self(self_),
			_builder(builder_) {}

		MenuBuilderSubItem(ref<const MenuBuilderSubItem>) = delete;

		MenuBuilderSubItem(mref<MenuBuilderSubItem>) = delete;

	public:
		ref<decltype(MenuItemData::subItems)::value_type> _self;
		ref<MenuBuilder> _builder;

	public:
		ref<MenuBuilderSubItem> setTitle(StringView title_) &;

		ref<MenuBuilderSubItem> setIcon(mref<TypedAssetGuid<engine::assets::TextureAsset>> icon_) &;

		ref<MenuBuilderSubItem> setAction(mref<std::function<void()>> action_) &;

		[[nodiscard]] MenuBuilderSubItem addSubItem(StringView itemId_);
	};

	struct MenuBuilderItem {
		constexpr MenuBuilderItem(StringView itemId_, ref<MenuBuilder> builder_) noexcept :
			_itemId(itemId_),
			_builder(builder_) {}

		MenuBuilderItem(ref<const MenuBuilderItem>) = delete;

		MenuBuilderItem(mref<MenuBuilderItem>) = delete;

	public:
		StringView _itemId;
		ref<MenuBuilder> _builder;

	public:
		ref<MenuBuilderItem> setTitle(StringView title_) &;

		ref<MenuBuilderItem> setIcon(mref<TypedAssetGuid<engine::assets::TextureAsset>> icon_) &;

		ref<MenuBuilderItem> setAction(mref<std::function<void()>> action_) &;

		[[nodiscard]] MenuBuilderSubItem addSubItem(StringView itemId_);
	};

	/**/

	class MenuBuilder {
	public:
		friend struct MenuBuilderItem;
		friend struct MenuBuilderSubItem;

	private:
		StableUnorderedMap<String, MenuItemData> _items;

	public:
		[[nodiscard]] MenuBuilderItem addItem(StringView itemId_);

		[[nodiscard]] MenuBuilderSubItem addSubItem(StringView parentItemId_, StringView itemId_);

	private:
		[[nodiscard]] SharedPtr<MenuItem> setupMenuItem(mref<SharedPtr<MenuItem>> item_, ref<const MenuItemData> data_) const;

	public:
		[[nodiscard]] SharedPtr<Menu> construct() const;
	};
}
