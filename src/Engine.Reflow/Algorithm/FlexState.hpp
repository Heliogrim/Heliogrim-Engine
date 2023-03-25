#pragma once

#include "FlexBox.hpp"
#include "FlexLine.hpp"

namespace hg::engine::reflow::algorithm {
    struct FlexState {
        FlexBox box;
        Vector<FlexLine> lines;
    };
}
