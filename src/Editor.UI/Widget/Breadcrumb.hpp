#pragma once

#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Filesystem/Path.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>

namespace hg::editor::ui {
	struct BreadcrumbEntry {
		AssocKey<string> key;
		wptr<engine::reflow::Widget> widget;
	};

	class Breadcrumb final :
		public engine::reflow::uikit::HorizontalLayout {
	public:
		using this_type = Breadcrumb;

		using action_fnc_type = std::function<void(cref<fs::Path>)>;

	public:
		Breadcrumb();

		~Breadcrumb() override;

	private:
		Vector<BreadcrumbEntry> _entries;

	public:
		void addNavEntry(cref<AssocKey<string>> key_, cref<string> title_, cref<fs::Path> value_);

		void removeNavEntry(cref<AssocKey<string>> key_);

		void clearNavEntries();

	private:
		u64 _areg;
		Vector<std::pair<u64, action_fnc_type>> _actions;

	private:
		void handleAction(cref<fs::Path> path_);

	public:
		u64 onAction(mref<action_fnc_type> fnc_);

		void offAction(u64 action_);
	};
}
