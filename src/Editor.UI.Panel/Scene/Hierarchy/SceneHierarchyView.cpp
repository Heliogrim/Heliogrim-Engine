#include "SceneHierarchyView.hpp"

#include <functional>
#include <Editor.UI.Service/Scene/Hierarchy/SceneHierarchyEntry.hpp>
#include <Heliogrim/Actor/Actor.hpp>

#include "SceneHierarchyController.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

SceneHierarchyView::SceneHierarchyView(
	ref<SceneHierarchyController> controller_,
	SharedPtr<uikit::Card> main_,
	std::weak_ptr<TreeView<service::SceneHierarchyEntry, std::type_identity_t>> tree_
) :
	_controller(controller_),
	main(::hg::move(main_)),
	tree(::hg::move(tree_)) {}

SharedPtr<uikit::Text> SceneHierarchyView::makeTreeItem(ref<const service::SceneHierarchyEntry> data_) const {
	switch (data_.type()) {
		case service::SceneHierarchyEntryType::eActor: {

			const auto actor = data_.value.as<nmpt<Actor>>();
			const auto guid = actor->guid();

			const auto text = make_sptr<uikit::Text>();
			text->setText(std::format(R"(Actor << {}-{}-{}-{} >>)", guid.pre, guid.c0, guid.c1, guid.post));

			return text;
		}
		case service::SceneHierarchyEntryType::eComponent: {

			const auto component = data_.value.as<nmpt<HierarchyComponent>>();
			const auto typeId = component->getTypeId();

			const auto text = make_sptr<uikit::Text>();
			text->setText(std::format(R"(Component << {} >>)", typeId.data));

			return text;
		}
		default: return nullptr;
	}
}

void SceneHierarchyView::displayTreeItems(
	ref<const Vector<service::SceneHierarchyEntry>> baseItems_,
	mref<std::function<void(ref<const service::SceneHierarchyEntry> parent_, ref<Vector<service::SceneHierarchyEntry>> children_)>>
	resolver_
) {

	if (tree.expired()) {
		return;
	}

	auto locked = tree.lock();
	locked->_generateFromData = [this](const auto& data_) { return makeTreeItem(data_); };
	locked->_resolveChildren = ::hg::move(resolver_);
	locked->_selectedFnc = [this](const auto& selectedSet_) {

		if (selectedSet_.empty()) {
			_controller.onSelectionChange({});
			return;
		}

		auto tmp = Vector<service::SceneHierarchyEntry> {};
		tmp.reserve(selectedSet_.size());
		for (auto&& selected : selectedSet_) { tmp.emplace_back(::hg::move(selected)); }
		_controller.onSelectionChange(tmp);
	};

	locked->setTreeViewSource(baseItems_);
}

UniquePtr<SceneHierarchyView> editor::ui::makeSceneHierarchyView(ref<SceneHierarchyController> controller_) {

	auto header = make_sptr<uikit::HorizontalLayout>();

	/**/

	const auto content = make_sptr<uikit::VScrollBox>();
	const auto tree = make_sptr<TreeView<service::SceneHierarchyEntry, std::type_identity_t>>();

	content->addChild(clone(tree));

	/**/

	auto wrapper = make_sptr<uikit::VerticalLayout>();
	wrapper->addChild(::hg::move(header));
	wrapper->addChild(::hg::move(content));

	auto card = uikit::makeCard({ .level = 1, .children = ::hg::move(wrapper) });

	return make_uptr<SceneHierarchyView>(
		controller_,
		::hg::move(card),
		tree
	);
}
