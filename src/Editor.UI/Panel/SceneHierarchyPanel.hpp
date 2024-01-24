#pragma once
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/VerticalPanel.hpp>
#include <Engine.Reflow/Widget/Tree/TreeView.hpp>
#include <Engine.Reflow/Widget/Scroll/VScrollBox.hpp>

#include "../Color/Dark.hpp"
#include "../Modules/SceneHierarchy.hpp"
#include "Engine.Reflow/ReflowUnit.hpp"

#if TRUE
#include "Engine.GFX.Glow.UI/TestUI.hpp"
#include "../Modules/SceneHierarchy/SceneViewEntry.hpp"
#include "Heliogrim/Actor.hpp"
#endif

namespace hg::editor::ui {
    class SceneHierarchyPanel :
        public engine::reflow::VerticalPanel {
    protected:
        SceneHierarchyPanel();

    private:
        sptr<engine::reflow::VScrollBox> _content;

    private:
    public:
        non_owning_rptr<SceneHierarchy> _module;
        non_owning_rptr<HierarchyResolverBase> _resolver;
        non_owning_rptr<HierarchyGeneratorBase> _generator;

    public:
        template <typename Type_>
        void setHierarchyTarget(
            const type_id typeId_,
            cref<Vector<typename engine::reflow::TreeView<Type_>::data_type>> sources_
        ) {

            using tree_view_type = engine::reflow::TreeView<Type_>;
            using data_type = typename tree_view_type::data_type;
            using resolver_type = HierarchyResolver<data_type, data_type>;
            using generator_type = HierarchyGenerator<data_type>;

            const auto resolver { _module->getResolver(typeId_) };
            const auto generator { _module->getGenerator(typeId_) };

            sptr<tree_view_type> tree {
                _content->children()->empty() ?
                    nullptr :
                    std::static_pointer_cast<tree_view_type, Widget>(_content->children()->front())
            };

            if (not tree || _resolver != resolver || _generator != generator) {

                assert(resolver);
                assert(generator);

                // Cleanup previous hierarchy and tree view
                tree.reset();
                _content->clearChildren();

                // Store new tree
                tree = make_sptr<tree_view_type>();
                tree->attr.width.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });
                tree->attr.maxWidth.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });
                tree->attr.height.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });
                tree->attr.maxHeight.setValue({ engine::reflow::ReflowUnitType::eAuto, 0.F });

                tree->attr.padding.setValue(engine::reflow::Padding { 4.F, 2.F });
                tree->attr.style.setValue(
                    engine::reflow::PanelStyle {
                        .backgroundColor = engine::color { 40.F, 44.F, 52.F, 255.F }
                    }
                );

                _content->addChild(tree);

                /**/

                _resolver = resolver;

                tree->_resolveChildren = [resolver](cref<data_type> item_, _Out_ ref<Vector<data_type>> children_) {
                    (*static_cast<const ptr<const resolver_type>>(resolver))(item_, children_);
                };

                /**/

                _generator = generator;

                tree->_generateFromData = [generator](cref<data_type> data_) {
                    return (*static_cast<const ptr<const generator_type>>(generator))(data_);
                };

                /**/

                tree->_selectedFnc = [](cref<typename tree_view_type::selected_set_type> data_) {
                    if (data_.empty()) {
                        return;
                    }

                    const auto& sel { *(data_.begin()) };
                    if (sel->type() == SceneViewEntryType::eActor) {
                        auto* actor { sel->template target<void>() };
                        storeEditorSelectedTarget(static_cast<const ptr<Actor>>(actor));
                        return;
                    }

                    if (sel->type() == SceneViewEntryType::eComponent) {
                        auto* actor { sel->template target<void>() };
                        storeEditorSelectedTarget(static_cast<const ptr<ActorComponent>>(actor));
                        return;
                    }
                };
            }

            /**/

            tree->setTreeViewSource(sources_);
        }

    public:
        static sptr<SceneHierarchyPanel> make(const non_owning_rptr<SceneHierarchy> module_);
    };
}
