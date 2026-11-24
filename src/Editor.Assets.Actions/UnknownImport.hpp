#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Editor.Action;
import Engine.Assets;
import Engine.Common;
import Engine.Storage.Registry;
#endif

namespace hg::editor::assets::actions {
	class UnknownImport :
		public InheritMeta<UnknownImport, SimpleAction> {
	public:
		UnknownImport(
			mref<engine::storage::FileUrl> localFileSource_,
			mref<engine::storage::FileUrl> importPath_
		);

		~UnknownImport() override;

	private:
		engine::storage::FileUrl _localFileSource;
		engine::storage::FileUrl _importPath;

		Vector<AssetGuid> _generatedAssets;
		Vector<engine::storage::StorageUrl> _generatedSources;

	public:
		[[nodiscard]] bool isReversible() const noexcept override;

	public:
		Result<void, std::runtime_error> apply() override;

		Result<void, std::runtime_error> revoke() override;

		Result<void, std::runtime_error> undo() override;
	};

	/**/

	[[nodiscard]] extern Result<Arci<UnknownImport>, std::runtime_error>
	makeUnknownWithExplorer(mref<engine::storage::FileUrl> importPath_);
}
