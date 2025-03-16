#include "PropertyView.hpp"

#include "Editor.UI.Service/Property/Property.hpp"

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
	for (const auto property : properties_) {
		auto insert = property->view(data_);
		locked->addChild(insert);
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
