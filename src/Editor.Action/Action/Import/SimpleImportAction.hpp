#pragma once
#include "ImportAction.hpp"

#include <Engine.Filesystem/Url.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Assets/Types/Asset.hpp>

namespace hg::editor {
	class SimpleImportAction :
		public ImportAction {
	public:
		using this_type = SimpleImportAction;

		inline constexpr static action_type_id typeId { "SimpleImportAction"_typeId };

	public:
		SimpleImportAction();

		SimpleImportAction(cref<fs::Url> source_, cref<fs::Url> target_);

		~SimpleImportAction();

	private:
		fs::Url _source;
		fs::Url _target;

		// TODO: ptr<void> _options;

	public:
		[[nodiscard]] fs::Url getSourceUrl() const noexcept;

		[[nodiscard]] fs::Url getTargetUrl() const noexcept;

	private:
		CompactSet<Arci<::hg::engine::assets::Asset>> _assets;

	public:
		[[nodiscard]] cref<decltype(_assets)> importedAssets() const noexcept;

	private:
		std::atomic_flag _running;
		std::atomic_flag _finished;

	protected:
		void setRunning();

		void setFinished();

	public:
		[[nodiscard]] bool isReversible() const noexcept override;

		[[nodiscard]] bool isPending() const noexcept override;

		[[nodiscard]] bool isRunning() const noexcept override;

		[[nodiscard]] bool isFinished() const noexcept override;

	public:
		void apply() override;

		void reverse() override;

	public:
		[[nodiscard]] operator ptr<await_signal_sub_type>() const noexcept;

	private:
		bool _failed;

	public:
		[[nodiscard]] bool failed() const noexcept override;
	};
}
