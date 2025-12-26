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
		bool beforeSpacer : 1;
		bool afterSpacer : 1;
		u16 precedence;

		String title;
		TypedAssetGuid<engine::assets::TextureAsset> icon;

		/**
		 * The action to invoke on menu item click event
		 * @details This callback action should return true to close and false to keep the menu open.
		 */
		std::function<bool()> action;

		/**/

		StableUnorderedMap<String, MenuItemData> subItems;
	};

	/**/

	struct MenuBuilderSubItem {
		constexpr MenuBuilderSubItem(ref<decltype(MenuItemData::subItems)::value_type> self_, ref<MenuBuilder> builder_) noexcept :
			_self(self_),
			_builder(builder_) {}

		MenuBuilderSubItem (ref<const MenuBuilderSubItem>) = delete;

		MenuBuilderSubItem (mref<MenuBuilderSubItem>) = delete;

	public:
		ref<decltype(MenuItemData::subItems)::value_type> _self;
		ref<MenuBuilder> _builder;

	public:
		ref<MenuBuilderSubItem> setTitle(StringView title_) &;

		ref<MenuBuilderSubItem> setIcon(mref<TypedAssetGuid<engine::assets::TextureAsset>> icon_) &;

		/**
		 * Will store the callback action for the sub menu item to generate
		 *
		 * @see MenuItemData::action
		 */
		ref<MenuBuilderSubItem> setAction(mref<std::function<bool()>> action_) &;

		[[nodiscard]] MenuBuilderSubItem addSubItem(StringView itemId_);
	};

	struct MenuBuilderItem {
		constexpr MenuBuilderItem(StringView itemId_, ref<MenuBuilder> builder_) noexcept :
			_itemId(itemId_),
			_builder(builder_) {}

		MenuBuilderItem (ref<const MenuBuilderItem>) = delete;

		MenuBuilderItem (mref<MenuBuilderItem>) = delete;

	public:
		StringView _itemId;
		ref<MenuBuilder> _builder;

	public:
		ref<MenuBuilderItem> setPrecedence(u16 precedence_) &;

		ref<MenuBuilderItem> setTitle(StringView title_) &;

		ref<MenuBuilderItem> setIcon(mref<TypedAssetGuid<engine::assets::TextureAsset>> icon_) &;

		/**
		 * Will store the callback action for the menu item to generate
		 *
		 * @see MenuItemData::action
		 */
		ref<MenuBuilderItem> setAction(mref<std::function<bool()>> action_) &;

		ref<MenuBuilderItem> setSpacer(bool before_, bool after_) &;

		[[nodiscard]] MenuBuilderSubItem addSubItem(StringView itemId_);
	};

	/**/

	class MenuBuilder {
	public:
		friend struct MenuBuilderItem;
		friend struct MenuBuilderSubItem;

	private:
		u16 _autoPrecedence = u16 { 1000uL };
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