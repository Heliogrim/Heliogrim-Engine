#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui::service {
	class PropertyService;
}

namespace hg::editor::ui::service {
	extern void initPropertyService(_Inout_ ref<PropertyService> service_);
}
