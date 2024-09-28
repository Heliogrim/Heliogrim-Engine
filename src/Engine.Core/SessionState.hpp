#pragma once

namespace hg::engine::core {
	class Session;
}

namespace hg::engine::core {
	class SessionState {
	public:
		friend class ::hg::engine::core::Session;

	public:
		using this_type = SessionState;

	protected:
		SessionState();

	public:
		~SessionState();
	};
}
