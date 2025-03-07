#pragma once

#include "../HorizontalPanel.hpp"
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>

namespace hg::engine::reflow {
	class TreeItem :
		public uikit::HorizontalLayout {
	public:
		using this_type = TreeItem;

    public:
        TreeItem();

        ~TreeItem() override;

    public:
        struct Attributes {
            DynamicAttribute<u32> level;
        } attr;

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
