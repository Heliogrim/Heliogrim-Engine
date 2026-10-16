#include "pch.h"

#include <algorithm>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Core/Module/SubModule.hpp>

using namespace hg;
using namespace hg::engine::core;

namespace Core {
	namespace {
		class StubSubModule final :
			public SubModule {
		public:
			StubSubModule(const string_view key_, CompactSet<SubModuleDependency> deps_ = {}) :
				SubModule(nullptr),
				_key(key_),
				_deps(std::move(deps_)) {}

			[[nodiscard]] DependencyKey moduleKey() const noexcept override { return _key; }

			[[nodiscard]] cref<CompactSet<SubModuleDependency>> dependencies() const noexcept override { return _deps; }

			void setup() override {}

			void start() override {}

			void stop() override {}

			void destroy() override {}

		private:
			string_view _key;
			CompactSet<SubModuleDependency> _deps;
		};

		[[nodiscard]] uptr<SubModule> make(const string_view key_, CompactSet<SubModuleDependency> deps_ = {}) {
			return make_uptr<StubSubModule>(key_, std::move(deps_));
		}

		[[nodiscard]] Vector<string_view> orderOf(cref<Modules> modules_) {
			Vector<string_view> keys {};
			for (const auto& module : modules_.getSubModules()) {
				keys.push_back(module->moduleKey());
			}
			return keys;
		}

		[[nodiscard]] bool orderIs(cref<Modules> modules_, std::initializer_list<string_view> expected_) {
			const auto keys = orderOf(modules_);
			return keys.size() == expected_.size() && std::equal(keys.begin(), keys.end(), expected_.begin());
		}
	}

	TEST(Modules, StartsEmpty) {
		Modules modules {};

		EXPECT_EQ(modules.getRootModuleCount(), 0uLL);
		EXPECT_EQ(modules.getCoreModuleCount(), 0uLL);
		EXPECT_EQ(modules.getSubModuleCount(), 0uLL);
		EXPECT_EQ(modules.validate(), DependencyValidationResult::eSuccess);
	}

	TEST(Modules, AddSubModuleFindsByKey) {
		Modules modules {};

		EXPECT_EQ(modules.addSubModule(make("a")), DependencyValidationResult::eSuccess);

		EXPECT_EQ(modules.getSubModuleCount(), 1uLL);
		EXPECT_NE(modules.getSubModule("a"), nullptr);
		EXPECT_EQ(modules.getSubModule("missing"), nullptr);
	}

	TEST(Modules, DuplicateKeyIsRejected) {
		Modules modules {};
		(void)modules.addSubModule(make("a"));

		EXPECT_EQ(modules.addSubModule(make("a")), DependencyValidationResult::eFailedUnique);
		EXPECT_EQ(modules.getSubModuleCount(), 1uLL);
	}

	TEST(Modules, SuccessorIsOrderedAfterItsDependency) {
		Modules modules {};
		(void)modules.addSubModule(make("a"));

		const auto res = modules.addSubModule(make("b", { SubModuleDependency { "a", SubModuleOrder::eSuccessor, true } }));

		EXPECT_EQ(res, DependencyValidationResult::eSuccess);
		EXPECT_TRUE(orderIs(modules, { "a", "b" }));
	}

	TEST(Modules, PredecessorIsOrderedBeforeItsDependency) {
		Modules modules {};
		(void)modules.addSubModule(make("a"));

		const auto res = modules.addSubModule(make("c", { SubModuleDependency { "a", SubModuleOrder::ePredecessor, true } }));

		EXPECT_EQ(res, DependencyValidationResult::eSuccess);
		EXPECT_TRUE(orderIs(modules, { "c", "a" }));
	}

	TEST(Modules, StoredModuleDependencyOnNewModuleIsHonored) {
		Modules modules {};
		// `b` is registered first, declaring it must come after `a`, which is not present yet.
		(void)modules.addSubModule(make("b", { SubModuleDependency { "a", SubModuleOrder::eSuccessor, true } }));
		(void)modules.addSubModule(make("a"));

		EXPECT_EQ(orderOf(modules).size(), 2uLL);
		const auto keys = orderOf(modules);
		const auto posA = std::find(keys.begin(), keys.end(), "a");
		const auto posB = std::find(keys.begin(), keys.end(), "b");
		EXPECT_LT(posA, posB);
	}

	TEST(Modules, MutualOrderIsRejected) {
		Modules modules {};
		// `a` wants to come after `c`, `c` wants to come after `a`.
		(void)modules.addSubModule(make("a", { SubModuleDependency { "c", SubModuleOrder::eSuccessor, false } }));

		const auto res = modules.addSubModule(make("c", { SubModuleDependency { "a", SubModuleOrder::eSuccessor, false } }));

		EXPECT_EQ(res, DependencyValidationResult::eFailedOrder);
		EXPECT_EQ(modules.getSubModuleCount(), 1uLL);
	}

	// LIMITATION: only direct a<->c contradictions are detected; transitive cycles (a < b, c before a and after b)
	// are inserted. Enable if cycle detection is extended.
	TEST(Modules, DISABLED_TransitiveOrderCycleIsRejected) {
		Modules modules {};
		(void)modules.addSubModule(make("a"));
		(void)modules.addSubModule(make("b", { SubModuleDependency { "a", SubModuleOrder::eSuccessor, true } }));

		const auto res = modules.addSubModule(
			make(
				"c",
				{
					SubModuleDependency { "a", SubModuleOrder::ePredecessor, true },
					SubModuleDependency { "b", SubModuleOrder::eSuccessor, true }
				}
			)
		);

		EXPECT_EQ(res, DependencyValidationResult::eFailedOrder);
	}


	TEST(Modules, ValidateReportsMissingRequiredDependency) {
		Modules modules {};
		(void)modules.addSubModule(make("b", { SubModuleDependency { "a", SubModuleOrder::eSuccessor, true } }));

		EXPECT_EQ(modules.validate(), DependencyValidationResult::eFailedRequired);

		(void)modules.addSubModule(make("a"));
		EXPECT_EQ(modules.validate(), DependencyValidationResult::eSuccess);
	}

	TEST(Modules, ValidateIgnoresMissingOptionalDependency) {
		Modules modules {};
		(void)modules.addSubModule(make("b", { SubModuleDependency { "a", SubModuleOrder::eSuccessor, false } }));

		EXPECT_EQ(modules.validate(), DependencyValidationResult::eSuccess);
	}

	TEST(Modules, RemoveSubModuleReportsSuccess) {
		Modules modules {};
		(void)modules.addSubModule(make("a"));
		const auto module = modules.getSubModule("a");

		EXPECT_TRUE(modules.removeSubModule(module));
		EXPECT_FALSE(modules.removeSubModule(module));
	}

	TEST(Modules, RemoveSubModuleErasesIt) {
		Modules modules {};
		(void)modules.addSubModule(make("a"));
		(void)modules.addSubModule(make("b"));

		(void)modules.removeSubModule(modules.getSubModule("a"));

		EXPECT_EQ(modules.getSubModule("a"), nullptr);
		EXPECT_NE(modules.getSubModule("b"), nullptr);
		EXPECT_EQ(modules.getSubModuleCount(), 1uLL);
	}
}
