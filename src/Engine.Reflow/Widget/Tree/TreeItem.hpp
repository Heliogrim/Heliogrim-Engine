#pragma once

#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>

namespace hg::engine::reflow {
	class TreeItem :
		public uikit::HorizontalLayout {
	public:
		using this_type = TreeItem;

	public:
		TreeItem();

		~TreeItem() override;

	private:
		u32 _level;
		bool _selected;

	public:
		[[nodiscard]] u32 getLevel() const noexcept;

		void setLevel(u32 level_);

		[[nodiscard]] bool isSelected() const noexcept;

		void setSelected(bool selected_);

	private:
		sptr<void> _treeDataItem;

	public:
		[[nodiscard]] sptr<void> getTreeDataItem() const noexcept;

		void setTreeDataItem(cref<sptr<void>> dataItem_);
	};
}
