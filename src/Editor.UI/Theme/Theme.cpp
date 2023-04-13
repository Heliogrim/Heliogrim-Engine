#include "Theme.hpp"

#include <Engine.Common/Make.hpp>

#include "DarkTheme.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

const non_owning_rptr<const Theme> Theme::get() {
    static sptr<Theme> theme = make_sptr<DarkTheme>();
    return theme.get();
}

Theme::~Theme() noexcept = default;
