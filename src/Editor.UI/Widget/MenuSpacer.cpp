#include "MenuSpacer.hpp"

#include <format>
#include <Engine.Reflow.Uikit/Atom/Paint.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

MenuSpacer::MenuSpacer() :
	BoxLayout() {}

MenuSpacer::~MenuSpacer() = default;

String MenuSpacer::getTag() const noexcept {
	return std::format(R"(MenuSpacer <{:#x}>)", reinterpret_cast<u64>(this));
}

/**/

SharedPtr<MenuSpacer> editor::ui::makeMenuSpacer() {

	auto spacer = make_sptr<MenuSpacer>();
	spacer->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
	spacer->getLayoutAttributes().update<attr::BoxLayout::padding>(Padding { 2.F });

	/**/

	auto render = make_sptr<uikit::Paint>(ReflowClassList { "[MenuSpacer] > line"sv }, nullptr);
	render->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
	render->getLayoutAttributes().update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 1.F });
	render->getLayoutAttributes().update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 1.F });

	/**/

	spacer->setChild(::hg::move(render));
	return spacer;
}
