#pragma once

#include "DataInputRate.hpp"
#include "TransferToken.hpp"
#include "TransferDataType.hpp"
#include "TransferType.hpp"

namespace hg::engine::gfx::acc {
    struct StageInput final {
        TransferToken token;
        TransferType transferType;
        TransferDataType dataType;
        /**/
        DataInputRate rate;
    };
}
