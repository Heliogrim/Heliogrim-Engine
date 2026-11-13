#pragma once

#include <array>
#include <atomic>
#include <filesystem>
#include <Engine.Async/Affine/AffineThread.hpp>
#include <Engine.Async/Future/Future.hpp>
#include <Engine.Async/Future/Promise.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Managed/Rc.hpp>

#include "IoBackend.hpp"
#include "IoFile.hpp"
#include "IoQueue.hpp"
#include "IoThread.hpp"
#include "IoTypes.hpp"

namespace hg::engine::io {
	class IoSystem final {
	public:
		friend class IoQueue;

	public:
		static constexpr u32 kMaxInFlight = 64;

	public:
		explicit IoSystem(IoBackendKind kind_ = IoBackendKind::eIocp, u32 maxInFlight_ = kMaxInFlight);

		IoSystem(const IoSystem&) = delete;

		IoSystem& operator=(const IoSystem&) = delete;

		// Queues must be released before the system is destroyed.
		~IoSystem();

	public:
		void start();

		void stop();

		[[nodiscard]] bool running() const noexcept;

	public:
		[[nodiscard]] Future<Arci<IoFile>, IoError> open(cref<std::filesystem::path> path_, IoOpenMode mode_);

		[[nodiscard]] Arci<IoQueue> createQueue(IoPriority priority_);

		[[nodiscard]] Future<u64, IoError> size(Arci<IoFile> file_);

		[[nodiscard]] Future<void, IoError> resize(Arci<IoFile> file_, u64 size_);

	private:
		IoThread _thread;
		uptr<IoBackend> _backend;
		u32 _maxInFlight;
		std::atomic_bool _running { false };

		// Io thread only.
		u32 _inFlight = 0;
		std::array<Vector<Arci<IoQueue>>, 4> _active;
		std::array<size_t, 4> _cursor {};

	private:
		[[nodiscard]] ref<async::AffineThread> thread() noexcept;

		void activate(mref<Arci<IoQueue>> queue_);

		[[nodiscard]] ptr<void> pick();

		// Issues while below the in-flight cap.
		void pump();
	};
}
