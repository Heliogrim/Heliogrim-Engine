#include "GpuTest.hpp"

import Engine.Common;

using namespace hg::test::gpu;
using namespace hg;

bool hg::test::gpu::validationLayerAvailable() {
	return std::ranges::contains(
		vk::enumerateInstanceLayerProperties(),
		StringView { "VK_LAYER_KHRONOS_validation" },
		[](const auto& layer_) { return StringView { layer_.layerName.data() }; }
	);
}

void GpuTest::SetUp() {

	if (not validationLayerAvailable()) {
		FAIL() << "validation layer not active";
	}

	_engine = make_uptr<HeadlessEngine>();
	_engine->onInstance = [this](const vk::Instance instance_) { _validation = make_uptr<ValidationCapture>(instance_); };
	_engine->beforeInstance = [this] { if (_validation) { _validation->release(); } };

	try {
		_engine->preInit();
		_engine->init();
		_engine->postInit();
	} catch (const std::exception& e_) {
		FAIL() << e_.what();
	}
}

void GpuTest::TearDown() {

	teardownDevice();

	if (not _validation) {
		return;
	}

	String listing {};
	for (const auto& message : _validation->messages()) {
		listing.append(message).append("\n");
	}
	EXPECT_TRUE(_validation->messages().empty()) << listing;
	_validation.reset();
}

sptr<engine::graphics::Device> GpuTest::device() const {
	return _engine ? _engine->getGraphics()->getCurrentDevice() : nullptr;
}

ref<ValidationCapture> GpuTest::validation() {
	return *_validation;
}

void GpuTest::teardownDevice() {

	if (not _engine) {
		return;
	}

	_engine->shutdown();
	_engine->exit();
	_engine.reset();
}
