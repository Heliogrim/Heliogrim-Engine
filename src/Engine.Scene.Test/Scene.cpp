#include "pch.h"
/**/

#include <string>
#include <vector>
#include <Engine.Scene/Scene.hpp>

using namespace hg::engine::scene;
using namespace hg;

namespace SceneModule {
	static std::vector<std::string> callLog {};

	template <int Id_>
	class RecordingSystem final :
		public SceneSystem<RecordingSystem<Id_>> {
	public:
		void prepare() override {
			callLog.push_back("prepare:" + std::to_string(Id_));
		}

		void cleanup() override {
			callLog.push_back("cleanup:" + std::to_string(Id_));
		}

		void broadcast(SceneBroadcastFlags flags_) override {
			callLog.push_back("broadcast:" + std::to_string(Id_) + ":" + std::to_string(flags_.unwrap));
		}

		void update(SceneUpdateFlags flags_) override {
			callLog.push_back("update:" + std::to_string(Id_) + ":" + std::to_string(flags_.unwrap));
		}

		void postprocess(ScenePostProcessFlags flags_) override {
			callLog.push_back("postprocess:" + std::to_string(Id_) + ":" + std::to_string(flags_.unwrap));
		}

		void add(const ptr<const MetaClass>, std::span<const ptr<SceneComponent>>) override {}

		void remove(const ptr<const MetaClass>, std::span<const ptr<const SceneComponent>>) override {}

		void clear() override {}
	};

	using SysA = RecordingSystem<0>;
	using SysB = RecordingSystem<1>;
	using SysC = RecordingSystem<2>;
	using SysUnused = RecordingSystem<3>;

	class TestScene final :
		public Scene<SysA, SysB, SysC> {
	public:
		[[nodiscard]] nmpt<IComponentRegisterContext> registerContext() noexcept override {
			return nullptr;
		}
	};

	/**/

	using Types = type_list<SysA, SysB, SysC>;

	static_assert(index_of_type<SysA, SysA, SysB, SysC>::value == 0);
	static_assert(index_of_type<SysC, SysA, SysB, SysC>::value == 2);
	static_assert(index_of_type<SysUnused, SysA, SysB, SysC>::value == -1);
	static_assert(std::is_same_v<type_of_index<1, SysA, SysB, SysC>::type, SysB>);
	static_assert(is_any_of_list<SysB, Types>::value);
	static_assert(not is_any_of_list<SysUnused, Types>::value);
	static_assert(index_of_packed_type<SysC, Types>::value == 2);
	static_assert(SceneHasSystem<TestScene, SysA>);
	static_assert(not SceneHasSystem<TestScene, SysUnused>);
	static_assert(TestScene::system_count == 3);

	/**/

	class SceneFixture : public ::testing::Test {
	protected:
		void SetUp() override {
			callLog.clear();
		}
	};

	TEST_F(SceneFixture, PrepareRunsForward) {
		TestScene scene {};
		scene.prepare();

		EXPECT_EQ(callLog, (std::vector<std::string> { "prepare:0", "prepare:1", "prepare:2" }));
	}

	TEST_F(SceneFixture, CleanupRunsBackward) {
		TestScene scene {};
		scene.cleanup();

		EXPECT_EQ(callLog, (std::vector<std::string> { "cleanup:2", "cleanup:1", "cleanup:0" }));
	}

	TEST_F(SceneFixture, DestructorCleansUpBackward) {
		{
			TestScene scene {};
		}

		EXPECT_EQ(callLog, (std::vector<std::string> { "cleanup:2", "cleanup:1", "cleanup:0" }));
	}

	TEST_F(SceneFixture, BroadcastReachesAllSystems) {
		TestScene scene {};
		scene.broadcast(SceneBroadcastFlagBits::ePostPhysics);

		const auto flag = std::to_string(static_cast<int>(SceneBroadcastFlagBits::ePostPhysics));
		EXPECT_EQ(
			callLog,
			(std::vector<std::string> { "broadcast:0:" + flag, "broadcast:1:" + flag, "broadcast:2:" + flag })
		);
	}

	TEST_F(SceneFixture, UpdateReachesAllSystems) {
		TestScene scene {};
		scene.update(SceneUpdateFlagBits::ePrePhysics);

		const auto flag = std::to_string(static_cast<int>(SceneUpdateFlagBits::ePrePhysics));
		EXPECT_EQ(callLog, (std::vector<std::string> { "update:0:" + flag, "update:1:" + flag, "update:2:" + flag }));
	}

	TEST_F(SceneFixture, PostprocessReachesAllSystems) {
		TestScene scene {};
		scene.postprocess(ScenePostProcessFlags { u8 { 1 } });

		EXPECT_EQ(
			callLog,
			(std::vector<std::string> { "postprocess:0:1", "postprocess:1:1", "postprocess:2:1" })
		);
	}

	TEST_F(SceneFixture, LookupByTypeAndIndex) {
		TestScene scene {};

		EXPECT_TRUE(scene.hasSceneSystem<SysA>());
		EXPECT_FALSE(scene.hasSceneSystem<SysUnused>());

		const auto b = scene.getSceneSystem<SysB>();
		EXPECT_EQ(b.get(), scene.getSceneSystem(1uLL).get());
		EXPECT_EQ(b.get(), scene.getSceneSystem<1uLL>().get());
		EXPECT_NE(static_cast<const void*>(scene.getSceneSystem<SysA>().get()), static_cast<const void*>(b.get()));
	}

	TEST_F(SceneFixture, LookupByIndexOutOfRange) {
		TestScene scene {};

		EXPECT_EQ(scene.getSceneSystem(3uLL).get(), nullptr);
		EXPECT_EQ(scene.getSceneSystem(~0uLL).get(), nullptr);
	}

	TEST_F(SceneFixture, LookupByMetaClass) {
		TestScene scene {};

		const auto* const metaB = SysB::getStaticMetaClass();
		const auto* const metaUnused = SysUnused::getStaticMetaClass();

		EXPECT_TRUE(scene.hasSceneSystem(metaB));
		EXPECT_FALSE(scene.hasSceneSystem(metaUnused));

		EXPECT_EQ(scene.getSceneSystem(metaB).get(), scene.getSceneSystem(1uLL).get());
		EXPECT_EQ(scene.getSceneSystem(metaUnused).get(), nullptr);
	}
}
