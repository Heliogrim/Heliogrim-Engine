#pragma once

#include "DataOutputRate.hpp"
#include "TransferDataType.hpp"
#include "TransferToken.hpp"
#include "TransferType.hpp"

namespace hg::engine::gfx::acc {
    struct StageOutput final {
        TransferToken token;
        TransferType transferType;
        TransferDataType dataType;
        /**/
        DataOutputRate rate;
    };
}
