#pragma once

#include "IoBackend.hpp"
#include "IoFile.hpp"
#include "IoQueue.hpp"
#include "IoThread.hpp"
#include "IoTypes.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Async;
import Engine.Common;
#endif

namespace hg::engine::io {
	class IoSystem final {
	public:
		friend class IoQueue;

	public:
		explicit IoSystem(const IoBackendKind kind_ = IoBackendKind::eIocp, const IoLimits limits_ = {});

		IoSystem(const IoSystem&) = delete;

		IoSystem& operator=(const IoSystem&) = delete;

		// Queues must be released before the system is destroyed.
		~IoSystem();

	public:
		// Only before start(); both limits must be non-zero.
		void setLimits(const IoLimits limits_);

		void start();

		void stop();

		[[nodiscard]] bool running() const noexcept;

		// Requests handed to the backend since construction.
		[[nodiscard]] u64 requestsIssued() const noexcept;

	public:
		[[nodiscard]] Future<Arci<IoFile>, IoError> open(ref<const std::filesystem::path> path_, const IoOpenMode mode_);

		[[nodiscard]] Arci<IoQueue> createQueue(const IoPriority priority_);

		[[nodiscard]] Future<u64, IoError> size(Arci<IoFile> file_);

		[[nodiscard]] Future<void, IoError> resize(Arci<IoFile> file_, const u64 size_);

	private:
		IoThread _thread;
		uptr<IoBackend> _backend;
		IoLimits _limits;
		std::atomic_bool _running { false };
		bool _started = false;
		// Set by the io thread once stopping left nothing in flight.
		std::atomic_bool _drained { false };
		std::atomic<u64> _requestsIssued { 0 };

		// Io thread only.
		bool _stopping = false;
		u32 _inFlight = 0;
		u64 _inFlightBytes = 0;
		std::array<Vector<Arci<IoQueue>>, 4> _active;
		std::array<size_t, 4> _cursor {};

	private:
		[[nodiscard]] ref<async::AffineThread> thread() noexcept;

		void activate(mref<Arci<IoQueue>> queue_);

		[[nodiscard]] ptr<void> pick();

		// Either in-flight limit reached.
		[[nodiscard]] bool saturated() const noexcept;

		// Issues while below both in-flight limits, at most one request cap per call; true if it may issue more right away.
		[[nodiscard]] bool pump();
	};
}
