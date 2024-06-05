#include "Action.hpp"

using namespace hg::editor;
using namespace hg;

action_type_id Action::getTypeId() const noexcept {
	return action_type_id { getMetaClass()->typeId() };
}
