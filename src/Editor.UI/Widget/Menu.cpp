#include "Menu.hpp"

#include <format>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

editor::ui::Menu::Menu() :
	CompoundWidget(ReflowClassList {}, nullptr) {}

editor::ui::Menu::~Menu() = default;

string editor::ui::Menu::getTag() const noexcept {
	return std::format(R"(Menu <{:#x}>)", reinterpret_cast<u64>(this));
}
