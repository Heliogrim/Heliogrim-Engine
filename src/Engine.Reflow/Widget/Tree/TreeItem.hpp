#pragma once

#include "../HorizontalPanel.hpp"

namespace hg::engine::reflow {
    class TreeItem :
        public HorizontalPanel {
    public:
        using this_type = TreeItem;

    public:
        TreeItem();

        ~TreeItem() override;

    private:
        bool _selected;

    public:
        [[nodiscard]] bool isSelected() const noexcept;

        void setSelected(const bool selected_);

    private:
        sptr<void> _treeDataItem;

    public:
        [[nodiscard]] sptr<void> getTreeDataItem() const noexcept;

        void setTreeDataItem(cref<sptr<void>> dataItem_);
    };
}
