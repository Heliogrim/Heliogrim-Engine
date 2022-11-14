#pragma once
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Widget/Panel.hpp>
#include <Engine.Reflow/Widget/Tree/TreeView.hpp>

#include "../Color/Dark.hpp"
#include "../Modules/SceneHierarchy.hpp"
#include "../Style/Style.hpp"

#if TRUE
#include "Engine.GFX.Glow.UI/TestUI.hpp"
#include "../Modules/SceneHierarchy/SceneViewEntry.hpp"
#include "Ember/Actor.hpp"
#endif

namespace ember::editor::ui {

    class SceneHierarchyPanel :
        public engine::reflow::Panel {
    protected:
        SceneHierarchyPanel();

    private:
        sptr<Box> _content;

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
                    _STD static_pointer_cast<tree_view_type, Widget>(_content->children()->front())
            };

            if (not tree || _resolver != resolver || _generator != generator) {

                assert(resolver);
                assert(generator);

                // Cleanup previous hierarchy and tree view
                tree.reset();
                _content->clearChildren();

                // Store new tree
                auto style {
                    engine::reflow::BoundStyleSheet::make(Style::get()->getStyleSheet(Style::AdoptFlexBoxKey))
                };
                style->maxHeight->type = engine::reflow::ReflowUnitType::eAuto;
                style->maxHeight->value = 0.F;
                style->reflowShrink = 0.F;
                style->reflowGrow = 0.F;
                style->color = color::Dark::backgroundInnerField;

                tree = make_sptr<tree_view_type>(_STD move(style));
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
