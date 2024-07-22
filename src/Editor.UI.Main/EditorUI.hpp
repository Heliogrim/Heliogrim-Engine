#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Core/Module/SubModule.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>

namespace hg::editor {
	class EditorEngine;
}

namespace hg::editor::ui {
	class AssetBrowser;
	class ObjectEditor;
	class SceneHierarchy;
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
		CompactSet<engine::core::SubModuleDependency> _dependencies;

	public:
		[[nodiscard]] engine::core::DependencyKey moduleKey() const noexcept override;

		[[nodiscard]] cref<CompactSet<engine::core::SubModuleDependency>> dependencies() const noexcept override;

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;

	private:
		sptr<ui::AssetBrowser> _assetBrowser;
		sptr<ui::ObjectEditor> _objectEditor;
		sptr<ui::SceneHierarchy> _sceneHierarchy;

	public:
		[[nodiscard]] sptr<ui::AssetBrowser> getAssetBrowser() const noexcept;

		[[nodiscard]] sptr<ui::ObjectEditor> getObjectEditor() const noexcept;

		[[nodiscard]] sptr<ui::SceneHierarchy> getSceneHierarchy() const noexcept;

	public:
		[[nodiscard]] nmpt<engine::assets::Font> getDefaultFont() const noexcept;

		nmpt<void> editorSelectedTarget = nullptr;

		// Warning: Error prone
		mutable smr<hg::engine::gfx::TextureResource> placeholderHolder = nullptr;

		[[nodiscard]] smr<engine::gfx::TextureResource> getPlaceholderImage() const noexcept;
	};
}
