#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../VBox.hpp"
#include "Engine.Common/Collection/BytellHashMap.hpp"

namespace ember::engine::reflow {

    class TreeViewItemBase {
    public:
        friend class TreeViewBase;

    public:
        using this_type = TreeViewItemBase;

    protected:
        TreeViewItemBase();

    public:
        virtual ~TreeViewItemBase() noexcept;

    protected:
        bool _expanded;
        wptr<Widget> _widget;
    };

    template <typename DataItemType_, template <typename> typename WrapType_ = sptr>
    class TreeViewItem :
        public TreeViewItemBase {
    public:
        using this_type = TreeViewItem<DataItemType_, WrapType_>;
        using data_type = WrapType_<DataItemType_>;

    public:
        TreeViewItem() :
            TreeViewItemBase() {}

        ~TreeViewItem() override = default;

    private:
        data_type _dataItem;

    public:
        [[nodiscard]] cref<data_type> getDataItem() const noexcept {
            return _dataItem;
        }
    };

    class TreeViewBase :
        public VBox {
    public:
        using this_type = TreeViewBase;

    protected:
        TreeViewBase();

    public:
        ~TreeViewBase() override;

    };

    template <typename DataItemType_, template <typename> typename WrapType_ = sptr>
    class TreeView :
        public TreeViewBase {
    public:
        using this_type = TreeView<DataItemType_, WrapType_>;
        using view_item_type = TreeViewItem<DataItemType_, WrapType_>;

        using data_type = typename view_item_type::data_type;
        using key_type = size_t;

        using generate_fnc_type = _STD function<sptr<Widget>(cref<data_type> data_)>;
        using selected_fnc_type = _STD function<void(cref<data_type> data_)>;
        using key_fnc_type = _STD function<key_type(cref<data_type> data_)>;

    public:
        TreeView();

        ~TreeView() override;

    private:
        ska::bytell_hash_map<key_type, view_item_type> _mapping;

    public:
        void insert(cref<data_type> data_);
    };

}
