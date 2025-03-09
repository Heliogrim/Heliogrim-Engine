#include "TreeItem.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>

using namespace hg::engine::reflow;
using namespace hg;

[[nodiscard]] bool styleIsSelected(cref<sptr<Widget>> widget_) {
	return static_cast<ptr<TreeItem>>(widget_.get())->isSelected();
}

TreeItem::TreeItem() :
	HorizontalLayout(),
	_level(),
	_selected(false) {}

TreeItem::~TreeItem() = default;

u32 TreeItem::getLevel() const noexcept {
	return _level;
}

void TreeItem::setLevel(u32 level_) {
	_level = level_;
	markAsPending();
}

bool TreeItem::isSelected() const noexcept {
	return _selected;
}

void TreeItem::setSelected(const bool selected_) {
	if (_selected != selected_) {
		this->markAsPending();
	}

	_selected = selected_;
}

sptr<void> TreeItem::getTreeDataItem() const noexcept {
	return _treeDataItem;
}

void TreeItem::setTreeDataItem(cref<sptr<void>> dataItem_) {
	_treeDataItem = dataItem_;
}
