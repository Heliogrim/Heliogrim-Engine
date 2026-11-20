#pragma once

#include <span>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Expected.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Diagnostics/Diagnostic.hpp>
#include <Engine.Diagnostics/Sink.hpp>

#include "ReadError.hpp"

namespace hg::engine::serialization {
	class LoadContext {
	public:
		inline constexpr static u32 maxDepth = 64uL;
		inline constexpr static u64 minBudget = 64uLL * 1024uLL * 1024uLL;

	public:
		explicit LoadContext(const u64 archiveSize_);

	private:
		struct Entry {
			ReadError error;
			String path;
		};

		Vector<String> _path;
		u32 _depth = 0;
		u64 _budget;

		bool _failed = false;
		Entry _first;
		Vector<Entry> _notes;
		Vector<Entry> _warnings;

		std::unordered_map<s64, std::unordered_set<String>> _read;

	public:
		/* The first failure latches, later ones become notes. */
		void fail(ReadError error_);

		[[nodiscard]] bool failed() const noexcept;

		void warn(ReadError error_);

	public:
		void pushPath(String segment_);

		void popPath();

		[[nodiscard]] String recordPath() const;

	public:
		[[nodiscard]] bool enterDepth();

		void leaveDepth();

		[[nodiscard]] bool charge(const u64 bytes_);

	public:
		void markRead(const s64 structOffset_, const StringView key_);

		[[nodiscard]] Vector<String> unreadKeys(const s64 structOffset_, std::span<const String> present_) const;

	public:
		/* Emits warnings to the sink; on a latched failure returns it (with the later ones as notes). */
		[[nodiscard]] Result<void, diag::Diagnostics> finish(
			ref<diag::DiagnosticSink> sink_,
			const StringView contextNote_
		);
	};
}
