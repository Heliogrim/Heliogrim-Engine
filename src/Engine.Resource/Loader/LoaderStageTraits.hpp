#pragma once

#include "__fwd.hpp"

namespace ember::engine::resource::loader {
    template <typename RequestType_, typename ResponseType_>
    struct LoaderStageTraits {
        using request_type = RequestType_;
        using request_value_type = typename request_type::value_type;
        using request_options_type = typename request_type::options_type;

        using stream_options_type = typename request_type::stream_type;

        using response_type = ResponseType_;
        using response_value_type = typename response_type::type;
    };
}