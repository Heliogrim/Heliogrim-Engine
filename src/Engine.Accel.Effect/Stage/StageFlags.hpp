#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
	enum class StageFlagBits : u8 {
		eVertex = 0b0000'0001,
		eFragment = 0b0000'0010,
		eCompute = 0b0000'0100,
		eGeometry = 0b0000'1000,
		eTessellationEval = 0b0001'0000,
		eTessellationCtrl = 0b0010'0000,
		/**/
		eAll = eVertex | eFragment | eCompute | eGeometry | eTessellationCtrl | eTessellationEval
	};

	typedef Flag<StageFlagBits> StageFlags;
}
