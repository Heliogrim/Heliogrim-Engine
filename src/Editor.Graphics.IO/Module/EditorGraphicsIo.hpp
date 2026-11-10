#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Core/Module/SubModule.hpp>

namespace hg::editor {
	class EditorGraphicsIo final :
		public engine::core::SubModule {
	public:
		using this_type = EditorGraphicsIo;

	public:
		EditorGraphicsIo(nmpt<engine::Engine> engine_);

		~EditorGraphicsIo() override;

	private:
		CompactSet<engine::core::SubModuleDependency> _dependencies;

	public:
		[[nodiscard]] engine::core::DependencyKey moduleKey() const noexcept override;

		[[nodiscard]] cref<CompactSet<engine::core::SubModuleDependency>> dependencies() const noexcept override;

	private:
		u64 _coreLoaderHookHandle;
		u64 _coreImporterHookHandle;

		u64 _editorTransformerHookHandle;

	private:
		void hookGraphicsEvents() noexcept;

		void unhookGraphicsEvents() noexcept;

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;
	};
}
