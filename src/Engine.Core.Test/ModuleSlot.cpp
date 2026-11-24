#include "pch.h"

/**/

import Engine.Common;
import Engine.Config;
import Engine.Core;
import Engine.Event;
import std;

using namespace hg;
using namespace hg::engine::core;

namespace Core {
	namespace {
		/* Only `CoreModule` needs an engine; it stores the address and never calls it. */
		class StubEngine final :
			public engine::Engine {
		public:
			bool preInit() override { return true; }

			bool init() override { return true; }

			bool postInit() override { return true; }

			bool start() override { return true; }

			bool stop() override { return true; }

			bool shutdown() override { return true; }

			bool exit() override { return true; }

		public:
			[[nodiscard]] ref<engine::Config> getConfig() const noexcept override { std::abort(); }

			[[nodiscard]] ref<GlobalEventEmitter> getEmitter() const noexcept override { std::abort(); }

			[[nodiscard]] ref<Modules> getModules() const noexcept override { std::abort(); }

			[[nodiscard]] ref<Timing> getTiming() const noexcept override { std::abort(); }

			[[nodiscard]] std::span<const nmpt<UniverseContext>> getUniverseContexts() const noexcept override {
				return {};
			}

			void addUniverse(mref<SharedPtr<Universe>>) override {}

			void removeUniverse(mref<SharedPtr<Universe>>) override {}
		};

		class SlotRoot final :
			public RootModule {
			friend class ::hg::engine::core::ModuleSlotAccess;

		private:
			static constinit std::atomic<ptr<SlotRoot>> _moduleSlot;

		public:
			int sentinel = 42;
		};

		constinit std::atomic<ptr<SlotRoot>> SlotRoot::_moduleSlot { nullptr };

		class SlotCore final :
			public CoreModule {
			friend class ::hg::engine::core::ModuleSlotAccess;

		private:
			static constinit std::atomic<ptr<SlotCore>> _moduleSlot;

		public:
			using CoreModule::CoreModule;

			void setup() override {}

			void start() override {}

			void stop() override {}

			void destroy() override {}

		public:
			int sentinel = 42;
		};

		constinit std::atomic<ptr<SlotCore>> SlotCore::_moduleSlot { nullptr };

		class ConstSlotRoot final :
			public RootModule {
			friend class ::hg::engine::core::ModuleSlotAccess;

		private:
			static constinit std::atomic<ptr<const ConstSlotRoot>> _moduleSlot;
		};

		constinit std::atomic<ptr<const ConstSlotRoot>> ConstSlotRoot::_moduleSlot { nullptr };

		class PlainRoot final :
			public RootModule {};

		constexpr DependencyKey slotCoreKey = "ModuleSlot::SlotCore";
	}

	TEST(ModuleSlot, ReadsNoneBeforeRegistration) {
		EXPECT_FALSE(engine::module<SlotRoot>().has_value());
		EXPECT_FALSE(engine::module<SlotCore>().has_value());
	}

	TEST(ModuleSlot, ReadsModuleAfterAddRoot) {
		Modules modules {};
		SlotRoot root;

		modules.addRootModule(root);
		const auto read = engine::module<SlotRoot>();

		ASSERT_TRUE(read.has_value());
		EXPECT_EQ(std::addressof(read.value()), std::addressof(root));
		EXPECT_EQ(read->sentinel, 42);

		modules.removeRootModule(root);
	}

	TEST(ModuleSlot, ReadsModuleAfterAddCore) {
		StubEngine stubEngine {};
		Modules modules {};
		SlotCore core { stubEngine };

		modules.addCoreModule(core, slotCoreKey);
		const auto read = engine::module<SlotCore>();

		ASSERT_TRUE(read.has_value());
		EXPECT_EQ(std::addressof(read.value()), std::addressof(core));
		EXPECT_EQ(read->sentinel, 42);

		modules.removeCoreModule(core);
	}

	TEST(ModuleSlot, ReadsNoneAfterRemove) {
		StubEngine stubEngine {};
		Modules modules {};
		SlotRoot root;
		SlotCore core { stubEngine };

		modules.addRootModule(root);
		modules.addCoreModule(core, slotCoreKey);

		modules.removeCoreModule(core);
		EXPECT_FALSE(engine::module<SlotCore>().has_value());

		modules.removeRootModule(root);
		EXPECT_FALSE(engine::module<SlotRoot>().has_value());
	}

	TEST(ModuleSlot, ConstSlotReturnsConstRef) {
		static_assert(std::is_same_v<decltype(engine::module<ConstSlotRoot>()), Opt<ref<const ConstSlotRoot>>>);

		Modules modules {};
		ConstSlotRoot root;

		modules.addRootModule(root);
		const auto read = engine::module<ConstSlotRoot>();

		ASSERT_TRUE(read.has_value());
		EXPECT_EQ(std::addressof(read.value()), std::addressof(root));

		modules.removeRootModule(root);
		EXPECT_FALSE(engine::module<ConstSlotRoot>().has_value());
	}

	TEST(ModuleSlot, TypeWithoutSlotRegistersWithoutPublishing) {
		static_assert(not ModuleSlotAccess::published<PlainRoot>);

		Modules modules {};
		PlainRoot root;

		modules.addRootModule(root);
		EXPECT_TRUE(modules.getRootModule<PlainRoot>().has_value());

		modules.removeRootModule(root);
	}

	TEST(ModuleSlot, DestroyingModulesRetractsSlots) {
		StubEngine stubEngine {};
		SlotRoot root;
		SlotCore core { stubEngine };

		{
			Modules modules {};
			modules.addRootModule(root);
			modules.addCoreModule(core, slotCoreKey);

			ASSERT_TRUE(engine::module<SlotRoot>().has_value());
			ASSERT_TRUE(engine::module<SlotCore>().has_value());
		}

		EXPECT_FALSE(engine::module<SlotRoot>().has_value());
		EXPECT_FALSE(engine::module<SlotCore>().has_value());
	}
}
