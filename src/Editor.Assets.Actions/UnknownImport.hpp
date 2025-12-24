#pragma once

#include <Editor.Action/Action/SimpleAction.hpp>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>
#include <Engine.Storage.Registry/Url/StorageUrl.hpp>

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