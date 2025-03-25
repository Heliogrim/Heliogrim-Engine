#include "ContextMenu.hpp"

#include <format>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

ContextMenu::ContextMenu(mref<SharedPtr<Menu>> menu_) :
	CompoundWidget(hg::move(menu_), ReflowClassList {}, nullptr) {}

ContextMenu::~ContextMenu() = default;

string ContextMenu::getTag() const noexcept {
	return std::format(R"(ContextMenu <{:#x}>)", reinterpret_cast<u64>(this));
}
