#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::editor::ui::service {
	class InputService;
}

namespace hg::editor::ui::service {
	extern void init_input_service(_Inout_ ref<InputService> service_);
}
