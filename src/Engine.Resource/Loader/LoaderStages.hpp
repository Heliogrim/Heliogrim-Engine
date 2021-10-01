#pragma once
#include <utility>

namespace ember::engine::res {

    /**
     * Loader Request
     */

    /**
     *
     */
    struct LoaderRequest {};

    /**
     * Source Loader
     */

    /**
     *
     */
    using SourceLoaderStageSource = LoaderRequest;

    /**
     *
     */
    struct SourceLoaderStageResult { };

    /**
     * Transform Loader
     */

    /**
     *
     */
    using TransformerStageSource = _STD pair<LoaderRequest, SourceLoaderStageResult>;

    /**
     *
     */
    struct TransformerStageResult { };

    /**
     * Feedback Loader
     */

    /**
     *
     */
    using FeedbackStageSource = _STD pair<LoaderRequest, TransformerStageResult>;

    /**
     *
     */
    struct FeedbackStageResult { };

    /**
     * Cache Loader
     */

    /**
     *
     */
    using CacheStageSource = _STD pair<LoaderRequest, FeedbackStageResult>;

    /**
     *
     */
    struct CacheStageResult { };

    /**
     * Loader Request
     */

    /**
     *
     */
    using LoaderRequestResult = _STD pair<LoaderRequest, CacheStageResult>;

}
