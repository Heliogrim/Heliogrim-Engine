#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui::service {
	class InputService;
}

namespace hg::editor::ui::service {
	extern void init_input_service(_Inout_ ref<InputService> service_);
}
