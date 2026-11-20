#pragma once

#include <gtest/gtest.h>

#include <Engine.Graphics/Device/Device.hpp>

#include "HeadlessEngine.hpp"
#include "ValidationCapture.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::test::gpu {
	[[nodiscard]] bool validationLayerAvailable();

	/**
	 * @details Brings up a headless device per test and fails the test on any validation warning or error.
	 */
	class GpuTest :
		public ::testing::Test {
	protected:
		uptr<HeadlessEngine> _engine;
		uptr<ValidationCapture> _validation;

	protected:
		void SetUp() override;

		void TearDown() override;

	public:
		[[nodiscard]] sptr<engine::graphics::Device> device() const;

		[[nodiscard]] ref<ValidationCapture> validation();

		/**
		 * @details Destroys the device and instance; idempotent, the captured messages stay readable.
		 */
		void teardownDevice();
	};
}
