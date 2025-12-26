#include "PropertyView.hpp"

#include <Editor.UI.Service/Property/Property.hpp>
#include <Editor.UI/Widget/Collapse.hpp>
#include <Engine.Reflow.Uikit/Atom/Paint.hpp>
#include <Engine.Reflow.Uikit/Atom/Text.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

PropertyView::PropertyView(
	ref<PropertyController> controller_,
	mref<SharedPtr<uikit::Card>> main_,
	mref<std::weak_ptr<uikit::VScrollBox>> container_
) :
	_controller(controller_),
	main(::hg::move(main_)),
	container(::hg::move(container_)) {}

void PropertyView::displayProperties(
	ref<const Vector<nmpt<const service::Property>>> properties_,
	ref<const Variant<nmpt<Actor>, nmpt<HierarchyComponent>>> data_
) {
	if (container.expired()) {
		return;
	}

	auto locked = container.lock();
	if (properties_.empty()) {
		locked->clearChildren();
		return;
	}

	locked->clearChildren();
	auto sections = std::map<StringView, SharedPtr<uikit::VerticalLayout>> {};

	for (const auto property : properties_) {

		auto sectionIt = sections.find(property->section);
		if (sectionIt == sections.end()) {

			auto section = make_sptr<Collapse>();
			section->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);

			auto sectionTitle = make_sptr<uikit::Stack>();
			sectionTitle->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
			sectionTitle->addChild(make_sptr<uikit::Paint>(ReflowClassList { "[PropertySectionHeader]"sv }, nullptr));

			auto title = make_sptr<uikit::Text>();
			title->setText(property->section);
			sectionTitle->addChild(::hg::move(title));

			/**/

			auto content = make_sptr<uikit::VerticalLayout>();
			std::get<0>(content->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
			std::get<0>(content->getLayoutAttributes().attributeSets).update<attr::BoxLayout::padding>(Padding { 4.F });

			section->setHeader(::hg::move(sectionTitle));
			section->setContent(clone(content));
			locked->addChild(::hg::move(section));

			sectionIt = sections.insert(std::make_pair(property->section, ::hg::move(content))).first;
		}

		auto insert = property->view(data_);
		sectionIt->second->addChild(insert);
	}
}

UniquePtr<PropertyView> editor::ui::makePropertyView(ref<PropertyController> controller_) {

	const auto container = make_sptr<uikit::VScrollBox>();
	std::get<0>(container->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
	std::get<0>(container->getLayoutAttributes().attributeSets).update<attr::BoxLayout::heightGrow>(1.F);
	std::get<0>(container->getLayoutAttributes().attributeSets).update<attr::BoxLayout::padding>(Padding { 8.F });

	auto card = uikit::makeCard({ .level = 1, .children = clone(container) });

	return make_uptr<PropertyView>(controller_, ::hg::move(card), container);
}
