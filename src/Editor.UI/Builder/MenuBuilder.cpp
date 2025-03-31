#include "MenuBuilder.hpp"

#include <Engine.Reflow.Uikit/Atom/Paint.hpp>
#include <Engine.Reflow.Uikit/Exp/Button.hpp>
#include <Widget/MenuSpacer.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

ref<MenuBuilderSubItem> MenuBuilderSubItem::setTitle(StringView title_) & {
	_self.second.title = ::hg::move(title_);
	return *this;
}

ref<MenuBuilderSubItem> MenuBuilderSubItem::setIcon(mref<TypedAssetGuid<engine::assets::TextureAsset>> icon_) & {
	_self.second.icon = ::hg::move(icon_);
	return *this;
}

ref<MenuBuilderSubItem> MenuBuilderSubItem::setAction(mref<std::function<void()>> action_) & {
	_self.second.action = ::hg::move(action_);
	return *this;
}

MenuBuilderSubItem MenuBuilderSubItem::addSubItem(StringView itemId_) {
	auto key = String { itemId_ };
	::hg::assertrt(not _self.second.subItems.contains(key));

	auto result = _self.second.subItems.insert_or_assign(::hg::move(key), MenuItemData {});
	::hg::assertrt(result.second);

	return MenuBuilderSubItem {
		*result.first,
		_builder
	};
}

ref<MenuBuilderItem> MenuBuilderItem::setPrecedence(u16 precedence_) & {
	const auto key = String { _itemId };
	const auto iter = _builder._items.find(key);

	iter->second.precedence = ::hg::move(precedence_);
	return *this;
}

ref<MenuBuilderItem> MenuBuilderItem::setTitle(StringView title_) & {
	const auto key = String { _itemId };
	const auto iter = _builder._items.find(key);

	iter->second.title = ::hg::move(title_);
	return *this;
}

ref<MenuBuilderItem> MenuBuilderItem::setIcon(mref<TypedAssetGuid<engine::assets::TextureAsset>> icon_) & {
	const auto key = String { _itemId };
	const auto iter = _builder._items.find(key);

	iter->second.icon = ::hg::move(icon_);
	return *this;
}

ref<MenuBuilderItem> MenuBuilderItem::setAction(mref<std::function<void()>> action_) & {
	const auto key = String { _itemId };
	const auto iter = _builder._items.find(key);

	iter->second.action = ::hg::move(action_);
	return *this;
}

ref<MenuBuilderItem> MenuBuilderItem::setSpacer(const bool before_, const bool after_) & {
	const auto key = String { _itemId };
	const auto iter = _builder._items.find(key);

	iter->second.beforeSpacer = before_;
	iter->second.afterSpacer = after_;

	return *this;
}

MenuBuilderSubItem MenuBuilderItem::addSubItem(const StringView itemId_) {
	return _builder.addSubItem(_itemId, itemId_);
}

MenuBuilderItem MenuBuilder::addItem(StringView itemId_) {

	auto key = String { itemId_ };
	::hg::assertrt(not _items.contains(key));

	const auto result = _items.insert_or_assign(::hg::move(key), MenuItemData { .precedence = (_autoPrecedence += 100uL) });
	::hg::assertrt(result.second);

	return MenuBuilderItem {
		itemId_,
		*this
	};
}

MenuBuilderSubItem MenuBuilder::addSubItem(StringView parentItemId_, StringView itemId_) {

	const auto primaryKey = String { parentItemId_ };
	auto iter = _items.find(primaryKey);
	::hg::assertrt(iter != _items.end());

	auto key = String { itemId_ };
	::hg::assertrt(not iter->second.subItems.contains(key));

	auto result = iter->second.subItems.insert_or_assign(::hg::move(key), MenuItemData {});
	::hg::assertrt(result.second);

	return MenuBuilderSubItem {
		*result.first,
		*this
	};
}

SharedPtr<MenuItem> MenuBuilder::setupMenuItem(mref<SharedPtr<MenuItem>> item_, ref<const MenuItemData> data_) const {

	auto text = make_sptr<uikit::Text>(ReflowClassList { "[MenuItem] > title"sv }, nullptr);
	text->getLayoutAttributes().update<attr::BoxLayout::padding>(Padding { 4.F, 2.F });
	text->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);

	auto content = uikit::makeButton(uikit::TextButtonCreateOptions { .level = 2u, .text = clone(text) });
	content->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);

	if (data_.icon.valid()) {
		const auto icon = make_sptr<uikit::Paint>();
		icon->getLayoutAttributes().update<attr::BoxLayout::minWidth>({ ReflowUnitType::eAbsolute, 16.F });
		icon->getLayoutAttributes().update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eAbsolute, 16.F });
		icon->getLayoutAttributes().update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 16.F });
		icon->getLayoutAttributes().update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 16.F });
	}

	if (not data_.title.empty()) {
		text->setText(data_.title);
	}

	if (data_.action != nullptr) {
		item_->onClick(
			[action = data_.action]([[maybe_unused]] const auto& event_) {
				action();
				return EventResponse::eHandled;
			}
		);
	}

	item_->setContent(::hg::move(content));

	/**/

	if (not data_.subItems.empty()) {

		auto menu = makeMenu();
		for (const auto& [key, value] : data_.subItems) {
			menu->addMenuItem(
				setupMenuItem(
					makeMenuItem(),
					value
				)
			);
		}

		item_->setSubMenu(::hg::move(menu));
	}

	/**/

	return item_;
}

SharedPtr<editor::ui::Menu> MenuBuilder::construct() const {

	auto menu = makeMenu();

	/**/

	auto sorted = Vector<nmpt<const MenuItemData>> {};
	for (const auto& [key, value] : _items) {
		const auto insertPoint = std::ranges::lower_bound(
			sorted,
			value.precedence,
			std::less<u16> {},
			[](const auto& item_) { return item_->precedence; }
		);
		sorted.insert(insertPoint, std::addressof(value));
	}

	for (const auto& item : sorted) {

		if (item->beforeSpacer) {
			menu->addMenuSpacer(makeMenuSpacer());
		}

		menu->addMenuItem(
			setupMenuItem(
				makeMenuItem(),
				*item
			)
		);

		if (item->afterSpacer) {
			menu->addMenuSpacer(makeMenuSpacer());
		}
	}

	/**/

	return menu;
}
