#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../VBox.hpp"
#include "Engine.Common/Collection/BytellHashMap.hpp"

namespace ember::engine::reflow {

    struct TreeViewItem {
    public:
        using this_type = TreeViewItem;

    public:
        bool expanded;

        bool children;
        bool expandedChildren;

        s64 parent;

        wptr<Widget> widget;
    };

    class TreeViewBase :
        public VBox {
    public:
        using this_type = TreeViewBase;

    protected:
        TreeViewBase(mref<sptr<BoundStyleSheet>> style_) :
            VBox(_STD move(style_)) {}

    public:
        ~TreeViewBase() override = default;

    };

    template <typename DataItemType_, template <typename> typename WrapType_ = sptr>
    class TreeView :
        public TreeViewBase {
    public:
        using this_type = TreeView<DataItemType_, WrapType_>;
        using view_item_type = TreeViewItem;

        using raw_data_type = DataItemType_;
        using data_type = WrapType_<raw_data_type>;

        using generate_fnc_type = _STD function<sptr<Widget>(cref<data_type> data_)>;
        using selected_fnc_type = _STD function<void(cref<data_type> data_)>;
        using resolve_fnc_type = _STD function<void(cref<data_type> item_, _Out_ ref<Vector<data_type>> children_)>;

    public:
        TreeView(mref<sptr<BoundStyleSheet>> style_) :
            TreeViewBase(_STD move(style_)),
            _linearizedData(),
            _linearizedView(),
            _mapping(),
            _resolveChildren(nullptr),
            _generateFromData(nullptr),
            _selectedFnc(nullptr),
            _sourceData() {}

        ~TreeView() override = default;

    private:
    public:
        Vector<data_type> _linearizedData;
        Vector<view_item_type> _linearizedView;

        ska::bytell_hash_map<data_type, view_item_type> _mapping;

        resolve_fnc_type _resolveChildren;
        generate_fnc_type _generateFromData;
        selected_fnc_type _selectedFnc;

    protected:
        [[nodiscard]] virtual const ptr<const decltype(_linearizedData)> sourceItems() const noexcept {
            return &_linearizedData;
        }

    protected:
        Vector<data_type> _sourceData;

    public:
        void setTreeViewSource(cref<Vector<data_type>> source_) {

            _sourceData.clear();
            _sourceData.shrink_to_fit();

            _sourceData.reserve(source_.size());
            _sourceData.insert(_sourceData.end(), source_.begin(), source_.end());

            /**/

            refreshTreeView();
        }

    private:
        void refreshTreeView() {

            refreshTreeViewData();
            regenerateViews();
        }

        void refreshTreeViewData() {

            // decltype(_linearizedData) nextData {}; // Don't need to duplicate, cause we can reuse local variable
            // decltype(_linearizedView) nextView {}; // Don't need to duplicate, cause we can reuse local variable
            decltype(_mapping) nextMapping {};

            nextMapping.reserve(_mapping.size());

            resolveItemPacking(_sourceData, -1, true, _linearizedData, _linearizedView, nextMapping);

            /**/

            // _linearizedData = _STD move(nextData);
            // _linearizedView = _STD move(nextView);
            _mapping = _STD move(nextMapping);
        }

        [[nodiscard]] bool resolveItemPacking(
            cref<Vector<data_type>> items_,
            const s64 parentIdx_,
            const bool store_,
            _Inout_ ref<decltype(_linearizedData)> dataPacking_,
            _Inout_ ref<decltype(_linearizedView)> viewPacking_,
            _Inout_ ref<decltype(_mapping)> mapPacking_
        ) {

            bool requireSubView { false };

            Vector<data_type> itemChildren {};
            for (const auto& item : items_) {

                itemChildren.clear();
                _resolveChildren(item, itemChildren);

                /**/

                ptr<view_item_type> view = nullptr;
                {
                    const auto it { _mapping.find(item) };
                    if (it != _mapping.end()) {
                        view = &it->second;
                    }
                }

                // TODO: resolve previous state to maintain selection and expanded items
                const bool hasChildren { not itemChildren.empty() };
                const bool isViewExpanded { view ? view->expanded : true/*false*/ };
                bool isSubViewExpanded { view ? view->expandedChildren : false };

                /**/

                if (store_) {

                    // Only add items which are reachable (`not store_` will relate to state "saving"/forwarding)
                    dataPacking_.push_back(item);
                    viewPacking_.push_back({ isViewExpanded, hasChildren, isSubViewExpanded, parentIdx_, {} });
                }

                /**/

                if (isViewExpanded || isSubViewExpanded) {

                    /* TODO: Check how we want to handle collapsed items, which were expanded before */
                    // TODO: If we want to "save" the expanded state for any item of the view, we need to forward the previous state without adding the actual element to the linearized data

                    isSubViewExpanded = resolveItemPacking(
                        itemChildren,
                        dataPacking_.size() - 1,
                        store_ && isViewExpanded,
                        dataPacking_,
                        viewPacking_,
                        mapPacking_
                    );

                }

                /**/

                if (isViewExpanded || isSubViewExpanded) {

                    // We don't need to save states for not-expanded elements (default)
                    mapPacking_.insert_or_assign(item, view_item_type {
                        isViewExpanded,
                        hasChildren,
                        isSubViewExpanded,
                        parentIdx_,
                        {}
                    });
                }

                /**/

                requireSubView = requireSubView || isViewExpanded || isSubViewExpanded;
            }

            return requireSubView;
        }

        void regenerateViews() {

            this->clearChildren();

            const auto size { _linearizedData.size() };
            for (u64 idx { 0ui64 }; idx < size; ++idx) {

                const auto& curData { _linearizedData[idx] };
                auto& curView { _linearizedView[idx] };

                const auto widget { _generateFromData(curData) };

                /**/

                if (not widget) {
                    continue;
                }

                /**/

                curView.widget = wptr<Widget> { widget };
                this->addChild(widget);

                const auto mapIt { _mapping.find(curData) };
                if (mapIt != _mapping.end()) {
                    mapIt->second.widget = widget;
                }
            }
        }
    };

}
