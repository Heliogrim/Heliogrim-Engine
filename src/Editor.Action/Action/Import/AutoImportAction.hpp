#pragma once

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>

#include "ImportAction.hpp"

namespace hg::editor {
	class AutoImportAction :
		public InheritMeta<AutoImportAction, ImportAction> {
	public:
		AutoImportAction() noexcept;

		AutoImportAction(_In_ mref<engine::storage::FileUrl> importFile_) noexcept;

		~AutoImportAction() override;

	private:
		engine::storage::FileUrl _importFile;
		Vector<Arci<engine::assets::Asset>> _generatedAssets;

	public:
		[[nodiscard]] engine::storage::FileUrl getImportFile() const noexcept;

		[[nodiscard]] cref<decltype(_generatedAssets)> generatedAssets() const noexcept;

	private:
		std::atomic_flag _running;
		std::atomic_flag _finished;
		bool _failed;

	private:
		void setRunning();

		void setFinished();

	public:
		[[nodiscard]] bool isReversible() const noexcept override;

		[[nodiscard]] bool isPending() const noexcept override;

		[[nodiscard]] bool isRunning() const noexcept override;

		[[nodiscard]] bool isFinished() const noexcept override;

	public:
		Result<void, std::runtime_error> apply() override;

		Result<void, std::runtime_error> revoke() override;

		Result<void, std::runtime_error> undo() override;

	public:
		[[nodiscard]] operator ptr<await_signal_sub_type>() const noexcept override;
	};
}
