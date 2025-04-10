#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Core/Module/SubModule.hpp>

#include "EditorUiModel.hpp"
#include "EditorUiServices.hpp"

namespace hg {
	struct asset_guid;
}

namespace hg::editor {
	class EditorEngine;
}

namespace hg::editor::ui {
	class SceneEditorController;
}

namespace hg::engine::assets {
	class Font;
}

namespace hg::editor {
	class EditorUI final :
		public engine::core::SubModule {
	public:
		using this_type = EditorUI;

	public:
		explicit EditorUI(_In_ ref<EditorEngine> engine_);

		~EditorUI() override;

	private:
		static Opt<ref<EditorUI>> _cached;
		CompactSet<engine::core::SubModuleDependency> _dependencies;

	public:
		[[nodiscard]] static Opt<ref<EditorUI>> getEditorUI();

		[[nodiscard]] engine::core::DependencyKey moduleKey() const noexcept override;

		[[nodiscard]] cref<CompactSet<engine::core::SubModuleDependency>> dependencies() const noexcept override;

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;

	private:
		ui::EditorUiModel _uiModel;
		ui::EditorUiServices _uiServices;

	private:
	public:
		UniquePtr<ui::SceneEditorController> _sceneEditorController;

	public:
		[[nodiscard]] ref<ui::EditorUiModel> getEditorModel() noexcept;

		[[nodiscard]] ref<const ui::EditorUiModel> getEditorModel() const noexcept;

		[[nodiscard]] ref<const ui::EditorUiServices> getEditorServices() const noexcept;

	public:
		[[nodiscard]] ref<const asset_guid> getDefaultFontGuid() const noexcept;

		[[nodiscard]] nmpt<engine::assets::Font> getDefaultFont() const noexcept;
	};
}
