#pragma once

#include <span>
#include <utility>

#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Expected.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Hash.hpp>
#include <Engine.Diagnostics/Diagnostic.hpp>
#include <Engine.Diagnostics/Sink.hpp>

#include "ReadError.hpp"

namespace hg::engine::serialization {
	class LoadContext {
	public:
		inline constexpr static u32 maxDepth = 64uL;
		inline constexpr static u64 minBudget = 64uLL * 1024uLL * 1024uLL;

	public:
		/* `url_` names the loaded asset or level; diagnostics carry it in their location. */
		explicit LoadContext(const u64 archiveSize_, String url_ = {});

	private:
		struct Entry {
			ReadError error;
			String path;
		};

		String _url;
		Vector<String> _path;
		u32 _depth = 0;
		u64 _budget;

		bool _failed = false;
		Entry _first;
		Vector<Entry> _notes;
		Vector<Entry> _warnings;

		using read_key = std::pair<ptr<const void>, s64>;

		struct ReadKeyHash {
			[[nodiscard]] size_t operator()(ref<const read_key> key_) const noexcept {
				size_t seed = std::hash<ptr<const void>> {}(key_.first);
				hash::hash_combine(seed, std::hash<s64> {}(key_.second));
				return seed;
			}
		};

		/* Keyed by (archive, struct offset): scratch archives of migrated records reuse offsets. */
		DenseMap<read_key, DenseSet<String>, ReadKeyHash> _read;

	public:
		/* The first failure latches, later ones become notes. */
		void fail(ReadError error_);

		[[nodiscard]] bool failed() const noexcept;

		[[nodiscard]] ptr<const ReadError> firstError() const noexcept;

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
		void markRead(ptr<const void> archive_, const s64 structOffset_, const StringView key_);

		[[nodiscard]] Vector<String> unreadKeys(ptr<const void> archive_, const s64 structOffset_, std::span<const String> present_) const;

		/* Drops every read set of `archive_`; a dying scratch archive calls this so a reused address starts clean. */
		void forgetArchive(ptr<const void> archive_);

	public:
		/* Emits warnings to the sink; on a latched failure returns it (with the later ones as notes). */
		[[nodiscard]] Result<void, diag::Diagnostics> finish(
			ref<diag::DiagnosticSink> sink_,
			const StringView contextNote_
		);
	};
}
