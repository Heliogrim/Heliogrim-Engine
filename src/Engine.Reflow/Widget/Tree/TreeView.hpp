#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../Scroll/VScrollBox.hpp"
#include "../Widget.hpp"
#include "Engine.Common/Collection/BytellHashMap.hpp"
#include "Engine.Common/Collection/Set.hpp"
#include "Engine.Common/Types.hpp"

namespace hg::engine::reflow {
    class TreeItem;

    template <typename DataItemType_, template <typename> typename WrappingType_>
    struct TreeDataItemEqual;

    template <typename DataItemType_, template <typename> typename WrapperType_>
    struct TreeDataItemHash;
}

namespace hg::engine::reflow {
    enum class TreeViewSelectionMode {
        eSingle = 0x0,
        eMultiple = 0x1,
    };

    struct TreeViewItem {
    public:
        using this_type = TreeViewItem;

    public:
        bool expanded;

        bool children;
        bool expandedChildren;

        s64 parent;
        s64 depth;

        wptr<Widget> widget;
    };

    class __declspec(novtable) TreeViewBase :
        public VScrollBox {
    public:
        using this_type = TreeViewBase;

    protected:
        TreeViewBase() :
            VScrollBox() {}

    public:
        ~TreeViewBase() override = default;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    protected:
        /**
         * Will generate a row component of type TreeItem
         *
         * @author Julius
         * @date 12.11.2022
         *
         * @param view_ The view to take into account.
         * @param content_ The external generated content to wrap.
         *
         * @returns The generated TreeItem wrapping the content
         *
         * Attention: Return items is guaranteed to be TreeItem, but for call simplification dropped string typing
         */
        sptr<Widget> generateRow(cref<TreeViewItem> view_, cref<sptr<Widget>> content_);

        /**
         * Will change the TreeItem's selection state and handle update
         *
         * @author Julius
         * @date 12.11.2022
         *
         * @param item_ The Widget (TreeItem) to modify
         * @param selected_ Whether the widget is marked as selected element
         *
         * Attention: We expect item_ to be an actual TreeItem, but for call simplification dropped strong typing
         */
        void changeStateSelection(cref<sptr<Widget>> item_, const bool selected_);

    public:
        EventResponse onFocus(cref<FocusEvent> event_) override;

        EventResponse onBlur(cref<FocusEvent> event_) override;
    };

    template <typename DataItemType_, template <typename> typename WrapType_ = sptr>
    class TreeView :
        public TreeViewBase {
    public:
        using this_type = TreeView<DataItemType_, WrapType_>;
        using view_item_type = TreeViewItem;

        using raw_data_type = DataItemType_;
        using data_type = WrapType_<raw_data_type>;

        using data_equal_type = typename TreeDataItemEqual<DataItemType_, WrapType_>::type;
        using data_hash_type = typename TreeDataItemHash<DataItemType_, WrapType_>::type;

        using selected_set_type = CompactSet<data_type, data_hash_type, data_equal_type>;

        using generate_fnc_type = _STD function<sptr<Widget>(cref<data_type> data_)>;
        using selected_fnc_type = _STD function<void(cref<selected_set_type> selected_)>;
        using resolve_fnc_type = _STD function<void(cref<data_type> item_, _Out_ ref<Vector<data_type>> children_)>;

    public:
        TreeView() :
            TreeViewBase(),
            _requiresRefresh(),
            _linearizedData(),
            _linearizedView(),
            _mapping(),
            _resolveChildren(nullptr),
            _generateFromData(nullptr),
            _selectedFnc(nullptr),
            _selectionMode(TreeViewSelectionMode::eSingle),
            _selected(),
            _cursor(_STD make_pair(-1, nullptr)),
            _sourceData() {}

        ~TreeView() override = default;

    private:
        mutable bool _requiresRefresh;

    private:
        void requestRefresh() {
            _requiresRefresh = true;
            markAsPending();
        }

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override {
            if (_requiresRefresh) {
                const_cast<ptr<this_type>>(this)->refreshTreeView();
                _requiresRefresh = false;
            }

            return TreeViewBase::prefetchDesiredSize(state_, scale_);
        }

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override {
            return TreeViewBase::computeDesiredSize(passState_);
        }

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override {
            return TreeViewBase::applyLayout(state_, _STD move(ctx_));
        }

    private:
    public:
        Vector<data_type> _linearizedData;
        Vector<view_item_type> _linearizedView;

        ska::bytell_hash_map<
            data_type,
            view_item_type,
            data_hash_type,
            data_equal_type
        > _mapping;

        resolve_fnc_type _resolveChildren;
        generate_fnc_type _generateFromData;
        selected_fnc_type _selectedFnc;

    private:
        TreeViewSelectionMode _selectionMode;
        selected_set_type _selected;

    private:
        _STD pair<s64, data_type> _cursor;
        /**
         * TODO: To handle cursor selections with multi select mode we need to introduce another cursor to track the origin
         *  TODO: If we don't track the origin, we have no orientation or distinct choice whether we want to select or deselect the next element
         */

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
            refreshSelection();
            refreshCursor();
        }

        void refreshTreeViewData() {

            // decltype(_linearizedData) nextData {}; // Don't need to duplicate, cause we can reuse local variable
            _linearizedData.clear();
            // decltype(_linearizedView) nextView {}; // Don't need to duplicate, cause we can reuse local variable
            _linearizedView.clear();
            decltype(_mapping) nextMapping {};

            nextMapping.reserve(_mapping.size());

            resolveItemPacking(_sourceData, -1, 0, true, _linearizedData, _linearizedView, nextMapping);

            /**/

            // _linearizedData = _STD move(nextData);
            // _linearizedView = _STD move(nextView);
            _mapping = _STD move(nextMapping);
        }

        [[nodiscard]] bool resolveItemPacking(
            cref<Vector<data_type>> items_,
            const s64 parentIdx_,
            const s64 depth_,
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
                const bool isViewExpanded { view ? view->expanded : false };
                bool isSubViewExpanded { view ? view->expandedChildren : false };

                /**/

                if (store_) {

                    // Only add items which are reachable (`not store_` will relate to state "saving"/forwarding)
                    dataPacking_.push_back(item);
                    viewPacking_.push_back({ isViewExpanded, hasChildren, isSubViewExpanded, parentIdx_, depth_, {} });
                }

                /**/

                if (isViewExpanded || isSubViewExpanded) {

                    /* TODO: Check how we want to handle collapsed items, which were expanded before */
                    // TODO: If we want to "save" the expanded state for any item of the view, we need to forward the previous state without adding the actual element to the linearized data

                    isSubViewExpanded = resolveItemPacking(
                        itemChildren,
                        dataPacking_.size() - 1,
                        depth_ + 1,
                        store_ && isViewExpanded,
                        dataPacking_,
                        viewPacking_,
                        mapPacking_
                    );

                }

                /**/

                if (isViewExpanded || isSubViewExpanded) {

                    // We don't need to save states for not-expanded elements (default)
                    mapPacking_.insert_or_assign(
                        item,
                        view_item_type {
                            isViewExpanded,
                            hasChildren,
                            isSubViewExpanded,
                            parentIdx_,
                            depth_,
                            {}
                        }
                    );
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

                const auto wrapper { generateRow(curView, widget) };
                this->addChild(wrapper);

                const auto mapIt { _mapping.find(curData) };
                if (mapIt != _mapping.end()) {
                    mapIt->second.widget = widget;
                }
            }
        }

    private:
        [[nodiscard]] bool openTreeItem(const s64 idx_) {

            if (idx_ < 0 || idx_ >= _linearizedView.size()) {
                return false;
            }

            auto& view { _linearizedView[idx_] };
            if (view.expanded || not view.children) {
                return false;
            }

            /**/

            const auto& data { _linearizedData[idx_] };
            const auto mapIt { _mapping.find(data) };

            if (mapIt == _mapping.end()) {

                _mapping.insert_or_assign(
                    data,
                    view_item_type {
                        true,
                        view.children,
                        view.expandedChildren,
                        view.parent,
                        view.depth,
                        view.widget
                    }
                );

            } else {
                mapIt->second.expanded = true;
            }

            /**/

            view.expanded = true;
            // TODO: Handle expanded children flag
            requestRefresh();

            return true;
        }

        [[nodiscard]] bool closeTreeItem(const s64 idx_) {

            if (idx_ < 0 || idx_ >= _linearizedView.size()) {
                return false;
            }

            auto& view { _linearizedView[idx_] };
            if (not view.expanded) {
                return false;
            }

            /**/

            const auto& data { _linearizedData[idx_] };
            const auto mapIt { _mapping.find(data) };

            if (mapIt != _mapping.end()) {

                if (view.expandedChildren) {
                    mapIt->second.expanded = false;
                } else {
                    _mapping.erase(mapIt);
                }

            }

            /**/

            view.expanded = false;
            // TODO: Handle expanded children flag
            requestRefresh();

            return true;
        }

    private:
        [[nodiscard]] bool intersects(cref<math::vec2> off_, cref<math::vec2> size_, cref<math::vec2> point_) {

            if (point_.x < off_.x || point_.x >= (off_.x + size_.x)) {
                return false;
            }

            if (point_.y < off_.y || point_.y >= (off_.y + size_.y)) {
                return false;
            }

            return true;
        }

        [[nodiscard]] sptr<Widget> findTreeView(cref<math::vec2> point_) {

            if (this->children()->empty()) {
                return nullptr;
            }

            // TODO: This will break cause `innerSize()` was ment to return content size with overflow
            const auto contentSize { /*this->innerSize()*/_layoutState.layoutSize };
            const auto off { contentSize.y - point_.y };

            float approxRowSize { 20.F };
            if (not this->children()->empty()) {
                //const auto childOuter = this->children()->front()->outerSize();
                const auto childOuter = this->children()->front()->layoutState().layoutSize;
                approxRowSize = MAX(approxRowSize, childOuter.y);
            }

            const auto start { MIN(static_cast<u64>(off / approxRowSize), (this->children()->size() - 1)) };

            const auto cbeg { this->children()->begin() };
            const auto cend { this->children()->end() };

            const auto& children { *this->children() };

            math::vec2 size { /*children[start]->outerSize()*/children[start]->layoutState().layoutSize };
            math::vec2 offset { /*children[start]->screenOffset()*/children[start]->layoutState().layoutOffset };
            if (intersects(offset, size, point_)) {
                return children[start];
            }

            if (point_.y < offset.y) {

                const auto crend { children.rend() };
                for (auto it { children.rbegin() + (children.size() - start) }; it != crend; ++it) {
                    size = (*it)->layoutState().layoutSize;
                    offset = (*it)->layoutState().layoutOffset;

                    if (intersects(offset, size, point_)) {
                        return *it;
                    }
                }

            } else {

                for (auto it { children.begin() + start }; it != cend; ++it) {
                    size = (*it)->layoutState().layoutSize;
                    offset = (*it)->layoutState().layoutOffset;

                    if (intersects(offset, size, point_)) {
                        return *it;
                    }
                }

            }

            return nullptr;
        }

        void refreshSelection() {

            auto diff { _selected };
            for (const auto& entry : _linearizedData) {
                diff.erase(entry);
            }

            const auto prevSize { _selected.size() };
            for (const auto& entry : diff) {
                _selected.erase(entry);
            }

            changeUnderlyingSelection(_selected, true);

            if (prevSize != _selected.size()) {
                _selectedFnc(_selected);
            }
        }

        void changeUnderlyingSelection(cref<decltype(_selected)> changing_, const bool selected_) {

            const auto dataEnd { _linearizedData.end() };
            for (auto dataIt { _linearizedData.begin() }; dataIt != dataEnd; ++dataIt) {

                if (not changing_.contains(*dataIt)) {
                    continue;
                }

                const auto off { _STD distance(_linearizedData.begin(), dataIt) };
                const auto& data { _linearizedView[off] };

                if (data.widget.expired()) {
                    continue;
                }

                const auto content { data.widget.lock() };
                changeStateSelection(content->parent(), selected_);
            }

        }

        void handleSelection(const s64 next_, const bool adv_) {

            const auto& view { _linearizedView[next_] };
            const auto& data { _linearizedData[next_] };

            const bool isSelected { _selected.contains(data) };

            if (not adv_ || _selectionMode == TreeViewSelectionMode::eSingle) {

                if (isSelected && _selected.size() <= 1ui64) {
                    return;

                } else {

                    changeUnderlyingSelection(_selected, false);
                    _selected.clear();

                    changeStateSelection(view.widget.lock()->parent(), true);
                    _selected.insert(data);
                }

            } else {
                if (isSelected) {
                    changeStateSelection(view.widget.lock()->parent(), false);
                    _selected.erase(data);

                } else {
                    changeStateSelection(view.widget.lock()->parent(), true);
                    _selected.insert(data);
                }
            }

            _selectedFnc(_selected);
        }

        void handleExternalSelection(cref<sptr<Widget>> target_, const bool adv_) {

            const auto viewIt = _STD find_if(
                _linearizedView.begin(),
                _linearizedView.end(),
                [target_](cref<view_item_type> view_) {
                    return target_ == view_.widget.lock()->parent();
                }
            );

            if (viewIt == _linearizedView.end()) {
                return;
            }

            const auto off { _STD distance(_linearizedView.begin(), viewIt) };

            moveCursor(off);
            handleSelection(off, adv_);
        }

    private:
        void refreshCursor() {

            const auto dataIt {
                _STD find_if(
                    _linearizedData.begin(),
                    _linearizedData.end(),
                    [
                        target = cref<decltype(_cursor.second)> { _cursor.second },
                        equal = data_equal_type {}
                    ](cref<data_type> entry_) {
                        return equal(target, entry_);
                    }
                )
            };
            if (dataIt == _linearizedData.end()) {
                _cursor.first = -1;
                _cursor.second = nullptr;
                return;
            }

            const auto off { _STD distance(_linearizedData.begin(), dataIt) };

            _cursor.first = off;
            _cursor.second = *dataIt;
        }

        [[nodiscard]] s64 getPrevRootViewEntry(const s64 idx_) {

            if ((idx_ - 1) < 0 || idx_ >= _linearizedView.size()) {
                return -1;
            }

            auto next { idx_ - 1 };
            while (_linearizedView[next].parent >= 0 && next > 0) {
                next = _linearizedView[next].parent;
            }

            return next;
        }

        [[nodiscard]] s64 getPrevDepthViewEntry(const s64 idx_) {

            if ((idx_ - 1) < 0 || idx_ >= _linearizedView.size()) {
                return -1;
            }

            const auto& cur { _linearizedView[idx_] };
            if (cur.parent >= 0) {
                return cur.parent;
            }

            return getPrevRootViewEntry(idx_);
        }

        [[nodiscard]] s64 getPrevViewEntry(const s64 idx_) {

            if ((idx_ - 1) < 0 || idx_ >= _linearizedView.size()) {
                return -1;
            }

            return idx_ - 1;
        }

        [[nodiscard]] s64 getNextViewEntry(const s64 idx_) {

            if (idx_ < 0 || (idx_ + 1) >= _linearizedView.size()) {
                return _linearizedView.size() - 1;
            }

            return idx_ + 1;
        }

        void moveCursor(const s64 next_) {

            _cursor.first = next_;
            _cursor.second = _linearizedData[next_];

            /**/

            const auto& view { _linearizedView[next_] };
            if (!view.widget.expired()) {

                const auto widget { view.widget.lock() };
                //this->scrollTo(widget->screenOffset(), widget->outerSize());
                this->scrollTo(widget->layoutState().layoutOffset, widget->layoutState().layoutSize);
            }
        }

        void handleCursor(const s64 prev_, const s64 next_, const bool adv_) {

            if (prev_ == next_ || next_ < 0) {
                return;
            }

            if (not adv_ || _selectionMode == TreeViewSelectionMode::eSingle) {
                handleSelection(next_, false);
            }

            if (adv_ && _selectionMode == TreeViewSelectionMode::eMultiple) {
                handleSelection(next_, true);
            }

            moveCursor(next_);
        }

    public:
        EventResponse onMouseButtonDown(cref<MouseEvent> event_) override {

            if (event_._button != 0x1) {
                return EventResponse::eHandled;
            }

            /**/

            const auto widget {
                findTreeView(
                    math::vec2 { static_cast<float>(event_._pointer.x), static_cast<float>(event_._pointer.y) }
                )
            };
            if (!widget) {
                return EventResponse::eHandled;
            }

            handleExternalSelection(widget, event_._modifier & 0xC0 /* 0x40 || 0x80 */);
            return EventResponse::eConsumed;
        }

        EventResponse onKeyDown(cref<KeyboardEvent> event_) override {

            // 79 :: Right
            // 80 :: Left
            // 81 :: Down
            // 82 :: Up

            if (event_._key == 82) {

                const auto next { getPrevViewEntry(_cursor.first) };
                handleCursor(_cursor.first, next, event_._modifier & 0x3 /* 0x1 || 0x1 */);

                return TreeViewBase::onKeyDown(event_);

            } else if (event_._key == 81) {

                const auto next { getNextViewEntry(_cursor.first) };
                handleCursor(_cursor.first, next, event_._modifier & 0x3 /* 0x1 || 0x1 */);

                return TreeViewBase::onKeyDown(event_);

            } else if (event_._key == 80) {

                if (not closeTreeItem(_cursor.first)) {
                    const auto next { getPrevDepthViewEntry(_cursor.first) };
                    handleCursor(_cursor.first, next, event_._modifier & 0x3 /* 0x1 || 0x1 */);
                }

                return TreeViewBase::onKeyDown(event_);

            } else if (event_._key == 79) {

                if (not openTreeItem(_cursor.first)) {
                    const auto next { getNextViewEntry(_cursor.first) };
                    handleCursor(_cursor.first, next, event_._modifier & 0x3 /* 0x1 || 0x1 */);
                }

                return TreeViewBase::onKeyDown(event_);

            }

            return TreeViewBase::onKeyDown(event_);
        }
    };

    template <typename DataItemType_>
    struct TreeDataItemEqual<DataItemType_, sptr> :
        public _STD equal_to<DataItemType_> {
        using this_type = TreeDataItemEqual<DataItemType_, sptr>;
        using base_equal_type = _STD equal_to<DataItemType_>;

        using type = this_type;

        [[nodiscard]] bool operator()(
            cref<sptr<DataItemType_>> left_,
            cref<sptr<DataItemType_>> right_
        ) const noexcept {

            if (not left_ && not right_) {
                return true;
            }

            if (not left_ || not right_) {
                return false;
            }

            return static_cast<cref<base_equal_type>>(*this)(*left_, *right_);
        }
    };

    template <typename DataItemType_>
    struct TreeDataItemHash<DataItemType_, sptr> :
        public _STD hash<DataItemType_> {
        using this_type = TreeDataItemHash<DataItemType_, sptr>;
        using base_hash_type = _STD hash<DataItemType_>;

        using type = this_type;

        [[nodiscard]] size_t operator()(
            cref<sptr<DataItemType_>> value_
        ) const noexcept {

            if (not value_) {
                // TODO: What should hash return when unwrapped type is empty
                return 0ui64;
            }

            return static_cast<cref<base_hash_type>>(*this)(*value_);
        }
    };
}
