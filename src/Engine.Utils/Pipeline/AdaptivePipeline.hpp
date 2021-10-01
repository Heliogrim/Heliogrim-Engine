#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "AdaptivePipelineStage.hpp"
#include "Engine.Common/Collection/List.hpp"

namespace ember::engine::utils {

    namespace {
        template <u64 Idx_, u64 ArgIdx_, typename Type_, typename ...Args_>
        struct unpack_nth_indexed {
            using type = _STD conditional_t<
                Idx_ == ArgIdx_,
                Type_,
                unpack_nth_indexed<Idx_, ArgIdx_ + 1, Args_...>
            >;
        };

        #if ENV_MSVC
        template <u64 Idx_, typename ...Args_> requires (Idx_ < sizeof...(Args_))
        #else
        template <u64 Idx_, typename ...Args_>
        #endif
        struct unpack_nth {
            using type = typename unpack_nth_indexed<Idx_, 0, Args_...>::type;
        };

        template <u64 Idx_, typename ...Args_>
        using unpack_nth_t = typename unpack_nth<Idx_, Args_...>::type;

        template <u64 Idx_, typename... Args_> requires (sizeof...(Args_) >= 2)
        using typed_stage = ptr<pipeline::adaptive::AdaptivePipelineStage<
            unpack_nth_t<Idx_, Args_...>,
            unpack_nth_t<Idx_ + 1, Args_...>
        >>;
    }

    template <typename ...Args_>
    class AdaptivePipeline {
    public:
        using value_type = AdaptivePipeline<Args_...>;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        static constexpr u64 stage_count = sizeof...(Args_) - 2ui64;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        AdaptivePipeline() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        virtual ~AdaptivePipeline() noexcept = default;

    private:
        vector<ptr<pipeline::AtomicPipelineStage>> _stages { stage_count, nullptr };

    public:
        /**
         * Sets the given pipeline stage into the pipeline at given StageIdx_
         *
         * @tparam StageIdx_ Type of the stage index.
         * @param  stage_ The stage.
         */
        template <u64 StageIdx_>
        void set(typed_stage<StageIdx_, Args_...> stage_) {
            _stages[StageIdx_] = stage_;
        }

        /**
         * Gets the stored pipeline stage from stages_ by StageIdx_
         *
         * @tparam StageIdx_ Type of the stage index.
         *
         * @returns A cref&lt;typed_stage&lt;StageIdx_,Args_...&gt;&gt;
         */
        template <u64 StageIdx_>
        [[nodiscard]] cref<typed_stage<StageIdx_, Args_...>> get() {
            return _stages[StageIdx_];
        }
    };
}
