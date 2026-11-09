#pragma once

#include <atomic>
#include <cstdint>
#include <Engine.Async/Affine/AffineThread.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::io {
	class IoThread final {
	public:
		IoThread();

		IoThread(const IoThread&) = delete;

		IoThread& operator=(const IoThread&) = delete;

		~IoThread();

	public:
		// Packets with this key only wake the loop; any other key is not expected yet.
		static constexpr std::uintptr_t wake_key = ~std::uintptr_t {};

		// Creates the completion port (concurrency 1) and starts the affine thread "IO Thread".
		void start();

		// Idempotent and thread-safe; joins the thread. The port stays open until destruction, so late wakes from in-flight producers are harmless.
		void stop();

		[[nodiscard]] ref<async::AffineThread> thread() noexcept;

		// A HANDLE, declared as void* to keep <Windows.h> out of this header. Safe to read from any thread after start() returned until destruction (it does not change after start); nullptr before start.
		[[nodiscard]] void* port() const noexcept;

	private:
		async::AffineThread _thread;
		void* _port = nullptr;
		std::atomic_bool _running { false };
	};
}
