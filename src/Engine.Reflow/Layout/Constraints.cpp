#include "Constraints.hpp"

using namespace ember::engine::reflow::layout;
using namespace ember::engine::reflow;
using namespace ember;

bool layout::hasStyleChanged(cref<StyleKeyStack::set_type> prev_, cref<StyleKeyStack> next_) {

    /**/
    for (const auto& entry : prev_) {
        if (not next_.contains(entry)) {
            return true;
        }
    }

    /**/
    for (const auto& pair : next_.getSetList()) {
        for (const auto& entry : pair.second) {
            if (not prev_.contains(entry)) {
                return true;
            }
        }
    }

    return false;
}

bool layout::hasConstSize(cref<StyleSheet> style_) {

    if (style_.minHeight->type != ReflowUnitType::eAbsolute)
        return false;

    if (style_.maxHeight->type != ReflowUnitType::eAbsolute)
        return false;

    if (style_.minWidth->type != ReflowUnitType::eAbsolute)
        return false;

    if (style_.maxWidth->type != ReflowUnitType::eAbsolute)
        return false;

    return (style_.minHeight->value == style_.maxHeight->value) && (style_.minWidth->value == style_.maxWidth->value);
}
