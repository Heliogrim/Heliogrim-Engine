#include "SeqScopedSlot.hpp"

#include <Engine.Common/Make.hpp>

#include "RecordScopedSlot.hpp"
#include "SeqSlot.hpp"

using namespace ember::engine::serialization;
using namespace ember;

SeqScopedSlot::SeqScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
    ScopedSlot(_STD move(scopedState_), make_sptr<SeqSlot>(_STD move(state_))) {}

SeqScopedSlot::~SeqScopedSlot() = default;
