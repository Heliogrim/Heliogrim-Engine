#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <Engine.Async/Affine/AffineThread.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::io {
	// Opaque OVERLAPPED_ENTRY, keeps <Windows.h> out of this header.
	struct IoPortEntry;

	class IoThread final {
	public:
		// Packets with this key only wake the loop; every other key is a backend completion handed to the harvest.
		constexpr static std::uintptr_t wake_key = ~std::uintptr_t {};

	public:
		IoThread();

		IoThread(const IoThread&) = delete;

		IoThread& operator=(const IoThread&) = delete;

		~IoThread();

	public:
		// Receives every non-wake packet on the io thread. std::move_only_function, as UniqueFn is call-once.
		using harvest_type = std::move_only_function<void(ref<const IoPortEntry>)>;

		// Runs after each inbox drain; true polls the port instead of blocking, as more work is ready.
		using drained_type = std::move_only_function<bool()>;

	public:
		// Creates the completion port (concurrency 1) and starts the affine thread "IO Thread".
		// An empty harvest keeps the assert on any non-wake key.
		void start(mref<harvest_type> harvest_ = {}, mref<drained_type> drained_ = {});

		// Idempotent and thread-safe; joins the thread. The port stays open until destruction, so late wakes from in-flight producers are harmless.
		void stop();

		[[nodiscard]] ref<async::AffineThread> thread() noexcept;

		// A HANDLE, declared as void* to keep <Windows.h> out of this header. Safe to read from any thread after start() returned until destruction (it does not change after start); nullptr before start.
		[[nodiscard]] ptr<void> port() const noexcept;

	private:
		async::AffineThread _thread;
		ptr<void> _port = nullptr;
		harvest_type _harvest;
		drained_type _drained;
		std::atomic_bool _running { false };
	};
}
