#include "pch.h"

/**/

#include <string>
#include <vector>
#include <Engine.Resource.Archive/MemoryReadonlyArchive.hpp>
#include <Engine.Serialization/Load/LoadContext.hpp>
#include <Engine.Serialization/Scheme/ActorScheme.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Heliogrim/Actor/Actor.hpp>

using namespace hg::engine::serialization;
using namespace hg::engine::resource;
using namespace hg;

namespace ActorSchemeMalformedModule {
	using raw_bytes = std::vector<u8>;

	void putS64(ref<raw_bytes> out_, const s64 value_) {
		u8 tmp[sizeof(s64)];
		std::memcpy(tmp, &value_, sizeof(s64));
		out_.insert(out_.end(), tmp, tmp + sizeof(s64));
	}

	void putStr(ref<raw_bytes> out_, const std::string& value_) {
		out_.push_back(0x0F);
		putS64(out_, static_cast<s64>(value_.size()));
		out_.insert(out_.end(), value_.begin(), value_.end());
	}

	/* Any instantiate or hydrate call means a malformed record produced an actor. */
	struct NoActorAccess {
		template <typename = void>
		static ptr<Actor> instantiate(nmpt<engine::acs::Registry>, cref<ActorGuid>) {
			ADD_FAILURE() << "instantiate must not run for a malformed component seq";
			return nullptr;
		}

		static void hydrate(cref<StructScopedSlot>, ref<Actor>) {
			ADD_FAILURE() << "hydrate must not run for a malformed component seq";
		}
	};

	TEST(ActorSchemeMalformed, NegativeComponentCountYieldsNoActor) {
		raw_bytes payload {};
		putStr(payload, std::string { Symbols::Guid.str });
		putStr(payload, std::string(16uLL, '\x01'));
		putStr(payload, std::string { Symbols::Components.str });
		// eSeq whose u64 count (all ones) exceeds its 8-byte payload, so getRecordCount yields -1.
		payload.push_back(0xC0);
		putS64(payload, 8LL);
		putS64(payload, -1LL);

		raw_bytes raw { 0xB0 };
		putS64(raw, static_cast<s64>(payload.size()));
		raw.insert(raw.end(), payload.begin(), payload.end());

		auto bytes = Vector<_::byte>(raw.size());
		std::memcpy(bytes.data(), raw.data(), raw.size());
		auto view = std::span<_::byte> { bytes };
		auto archive = MemoryReadonlyArchive { view };
		auto ctx = LoadContext { raw.size() };

		const auto root = RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx };

		ptr<Actor> result = nullptr;
		EXPECT_NO_THROW((result = ActorScheme<Actor>::deserialize<NoActorAccess>(root, nullptr)));

		EXPECT_EQ(result, nullptr);
		EXPECT_TRUE(ctx.failed());
		ASSERT_NE(ctx.firstError(), nullptr);
		EXPECT_EQ(ctx.firstError()->kind, ReadError::Kind::eMalformed);
	}
}
