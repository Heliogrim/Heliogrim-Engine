#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Core/Module/SubModule.hpp>
#include <Engine.Common/Managed/Rc.hpp>

#include "Log/__fwd.hpp"

namespace hg::editor {
	class Action;
}

namespace hg::editor {
	constexpr static engine::core::DependencyKey ActionDepKey = { "::hg::editor::ActionManager"sv };

	/**/

	class ActionManager :
		public engine::core::SubModule {
	public:
		using this_type = ActionManager;

	public:
		explicit ActionManager(_In_ ref<engine::Engine> engine_);

		~ActionManager() override;

	private:
		CompactSet<engine::core::SubModuleDependency> _deps;

	public:
		[[nodiscard]] engine::core::DependencyKey moduleKey() const noexcept override;

		[[nodiscard]] cref<CompactSet<engine::core::SubModuleDependency>> dependencies() const noexcept override;

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;

	private:
		uptr<ActionLog> _log;
		uptr<ActionDispatcher> _dispatcher;

	public:
		cref<ActionManager> apply(_In_ mref<Arci<Action>> action_) const;

		template <CompleteType ActionType_> requires (not std::is_same_v<ActionType_, Action>)
		cref<ActionManager> apply(_In_ mref<Arci<ActionType_>> action_) const {
			return apply(std::move(action_).template into<Action>());
		}

		void revert() const;

		void reapply() const;
	};
}
