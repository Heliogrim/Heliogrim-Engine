#include "../pch.h"

/**/

#include <cstdlib>
#include <Engine.Core/Engine.hpp>
#include <Engine.Graphics/Graphics.hpp>
#include <Engine.Graphics/vkinc.hpp>

#ifdef WIN32
#include <Engine.Common/stdafx.h>
#endif

#include "../Harness/GpuTest.hpp"
#include "../Harness/HeadlessEngine.hpp"

import Engine.Common;

using namespace hg::test::gpu;
using namespace hg;

namespace {
	// Any size is reported as leaked; 16 bytes is just an arbitrary small one.
	inline constexpr static vk::DeviceSize leakBytes = 16uLL;
}

TEST_F(GpuTest, DeviceComesUp) {
	ASSERT_NE(device(), nullptr);
	EXPECT_NE(device()->graphicsQueue(), nullptr);
	EXPECT_EQ(engine::Engine::getEngine()->getGraphics()->getCurrentDevice(), device())
		<< device()->vkPhysicalDevice().getProperties().deviceName.data();
}

TEST_F(GpuTest, LeakIsDetected) {
	const vk::BufferCreateInfo info { {}, leakBytes, vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive };
	[[maybe_unused]] const auto leaked = device()->vkDevice().createBuffer(info);

	teardownDevice();

	const auto messages = validation().messages();
	String joined {};
	for (const auto& message : messages) {
		joined += message + '\n';
	}
	EXPECT_TRUE(std::ranges::any_of(messages, [](ref<const String> m_) { return m_.find("VkBuffer") != String::npos; })) << joined;
	validation().clear();
}

#ifdef WIN32
TEST(GpuTestEnv, UnknownGpuOverrideFails) {

	constexpr auto name = "HELIOGRIM_TEST_GPU";
	const ptr<const char> previous = std::getenv(name);

	struct Restore {
		Opt<String> value;

		~Restore() {
			// An empty value removes the variable again
			_putenv_s(name, value.has_value() ? value->c_str() : "");
		}
	} const restore { previous != nullptr ? SomeVal<String>(previous) : None };

	_putenv_s(name, "__no_such_gpu__");

	auto engine = make_uptr<HeadlessEngine>();
	engine->preInit();

	String what {};
	try {
		engine->init();
	} catch (const std::exception& e_) {
		what = e_.what();
	}

	engine->shutdown();
	engine->exit();
	engine.reset();

	EXPECT_NE(what.find("no GPU with these capabilities found"), String::npos) << what;
}
#endif

TEST(GpuTestEnv, ValidationLayerRequired) {
	EXPECT_TRUE(validationLayerAvailable());
}
