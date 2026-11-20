#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
	class AccelerationEffect;
	class Stage;
	class AccelerationEffectInputs;
	class AccelerationEffectBindings;
	class AccelerationEffectOutputs;
	struct StageInput;
	struct StageOutput;
	enum class TransferDataType : u8;
	enum class TransferType;
}
