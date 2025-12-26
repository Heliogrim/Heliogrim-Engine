#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::editor::ui::service {
	class PropertyService;
}

namespace hg::editor::ui::service {
	extern void initPropertyService(_Inout_ ref<PropertyService> service_);
}
