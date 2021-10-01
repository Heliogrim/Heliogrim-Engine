#pragma once

#include <Engine.Common/Concurrent/Future.hpp>

namespace ember::engine::utils::pipeline::adaptive {

    template <typename ValueType_, typename PushCtxType_, typename PullCtxType_>
    struct AdaptivePipelineNext {

        using value_type = ValueType_;
        using push_ctx_type = PushCtxType_;
        using pull_ctx_type = PullCtxType_;

        concurrent::future<value_type> store;

        push_ctx_type pushCtx;
        pull_ctx_type pullCtx;

    };
}
