#include "pch.h"

#include <atomic>
#include <Engine.GFX.Material/MaterialParameterStorage.hpp>
#include <Engine.GFX.Material/ParameterIdentifier.hpp>

using namespace hg;
using namespace hg::engine::gfx::material;

namespace GfxMaterial {
	TEST(MaterialParameterStorageTest, SetAndGetValue) {
		MaterialParameterStorage<int> storage {};

		storage.set(42);

		EXPECT_EQ(storage.get(), 42);
	}

	TEST(MaterialParameterStorageTest, LaterSetOverwrites) {
		MaterialParameterStorage<float> storage {};
		storage.set(1.F);

		storage.set(2.5F);

		EXPECT_EQ(storage.get(), 2.5F);
	}

	TEST(MaterialParameterStorageTest, AtomicStorageReturnsByValue) {
		MaterialParameterStorage<std::atomic<int>> storage {};

		storage.set(7);

		EXPECT_EQ(storage.get(), 7);
	}

	TEST(MaterialParameterStorageTest, PointerStorageKeepsPointer) {
		MaterialParameterStorage<int*> storage {};
		int target = 3;

		storage.set(&target);

		EXPECT_EQ(storage.get(), &target);
	}

	TEST(ParameterIdentifierTest, EqualityComparesData) {
		EXPECT_TRUE((ParameterIdentifier { 3u } == ParameterIdentifier { 3u }));
		EXPECT_FALSE((ParameterIdentifier { 3u } == ParameterIdentifier { 4u }));
	}
}
