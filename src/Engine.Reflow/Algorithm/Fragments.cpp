#include "Fragments.hpp"

#include "../ReflowUnit.hpp"
#include "../Style/StyleSheet.hpp"

using namespace hg::engine::reflow::algorithm;
using namespace hg::engine::reflow;
using namespace hg;

void algorithm::applyPaddingToOuter(cref<StyleSheet> style_, ref<math::vec2> target_) {
    target_.x -= style_.padding->x;
    target_.y -= style_.padding->y;
    target_.x -= style_.padding->z;
    target_.y -= style_.padding->w;
}

math::vec2 algorithm::calcImplicitInnerSize(
    cref<StyleSheet> style_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_
) {

    math::vec2 local {};

    /**/

    if (style_.width->type == ReflowUnitType::eRelative) {
        local.x = space_.x * style_.width->value;
    } else if (style_.width->type == ReflowUnitType::eAbsolute) {
        local.x = MIN(style_.width->value, space_.x);
    }

    if (style_.height->type == ReflowUnitType::eRelative) {
        local.y = space_.y * style_.height->value;
    } else if (style_.height->type == ReflowUnitType::eAbsolute) {
        local.y = MIN(style_.height->value, space_.y);
    }

    /**/

    if (style_.minWidth->type != ReflowUnitType::eAuto) {
        if (style_.minWidth->type == ReflowUnitType::eAbsolute) {
            local.x = MAX(local.x, style_.minWidth->value);
        } else if (style_.minWidth->type == ReflowUnitType::eRelative) {
            local.x = space_.x * style_.minWidth->value;
        }
    }

    if (style_.minHeight->type != ReflowUnitType::eAuto) {
        if (style_.minHeight->type == ReflowUnitType::eAbsolute) {
            local.y = MAX(local.y, style_.minHeight->value);
        } else if (style_.minHeight->type == ReflowUnitType::eRelative) {
            local.y = space_.y * style_.minHeight->value;
        }
    }

    /**/

    math::vec2 maxSize { limit_ };

    if (style_.maxWidth->type != ReflowUnitType::eAuto) {
        if (style_.maxWidth->type == ReflowUnitType::eRelative) {
            maxSize.x = MIN(maxSize.x, style_.maxWidth->value * space_.x);
        } else if (style_.maxWidth->type == ReflowUnitType::eAbsolute) {
            maxSize.x = MIN(maxSize.x, style_.maxWidth->value);
        }
    }

    if (style_.maxHeight->type != ReflowUnitType::eAuto) {
        if (style_.maxHeight->type == ReflowUnitType::eRelative) {
            maxSize.y = MIN(maxSize.y, style_.maxHeight->value * space_.y);
        } else if (style_.maxHeight->type == ReflowUnitType::eAbsolute) {
            maxSize.y = MIN(maxSize.y, style_.maxHeight->value);
        }
    }

    /**/

    return local = compMin<float>(local, maxSize);
}
