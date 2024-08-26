#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::core {
	class Session {
	public:
		using this_type = Session;

	public:
		Session();

		Session(cref<this_type>) = delete;

		Session(mref<this_type>) noexcept = delete;

		~Session();

	private:
		uptr<class SessionState> _state;
		uptr<class UniverseContext> _universeContext;

	public:
		[[nodiscard]] ref<class SessionState> getState() const noexcept;

		[[nodiscard]] ref<class UniverseContext> getUniverseContext() const noexcept;
	};
}
