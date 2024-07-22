#pragma once
#include <filesystem>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Reflow/Widget/__fwd.hpp>
#include <Engine.Reflow/Widget/VerticalPanel.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>

namespace hg::editor::ui {
	class AssetBrowser;
}

namespace hg::editor::ui {
	class AssetBrowserPanel :
		public engine::reflow::VerticalPanel {
	protected:
		AssetBrowserPanel();

	public:
		~AssetBrowserPanel() override;

		//private:
	public:
		non_owning_rptr<AssetBrowser> _browser;

		fs::Url _browserRoot;
		fs::Url _browserCwd;

	public:
		void changeCwd(cref<fs::Url> nextCwd_);

	private:
		sptr<engine::reflow::HorizontalPanel> _nav;
		sptr<engine::reflow::UniformGridPanel> _items;

	private:
		[[nodiscard]] sptr<engine::reflow::HorizontalPanel> getNavContainer() const;

		[[nodiscard]] sptr<engine::reflow::UniformGridPanel> getItemContainer() const;

		void dropNav();

		void buildNav();

		void dropItems();

		void buildItems();

	private:
		wptr<engine::reflow::Popup> _dialog;

	private:
		void closeImportDialog();

		void openImportDialog(cref<fs::Url> fqUrlSource_);

	public:
		engine::reflow::EventResponse onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) override;

		engine::reflow::EventResponse onDrop(cref<engine::reflow::DragDropEvent> event_) override;

	public:
		static sptr<AssetBrowserPanel> make(const non_owning_rptr<AssetBrowser> browser_, cref<fs::Url> root_);
	};
}
