#pragma once
#include <Engine.Common/Concurrent/Future.hpp>

namespace ember::engine::utils::pipeline::adaptive {
    template <typename Type_>
    class AdaptivePipelineResult :
        protected concurrent::future<_STD remove_cvref_t<Type_>> {
    private:
        friend class AdaptivePipeline;

    public:
        using base_type = concurrent::future<_STD remove_cvref_t<Type_>>;

        using value_type = _STD remove_cvref_t<Type_>;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using this_type = AdaptivePipelineResult<Type_>;
        using reference_this_type = ref<this_type>;
        using const_reference_this_type = cref<this_type>;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        AdaptivePipelineResult() = delete;

        /**
         * Constructor
         *
         * @author Julius
         * @date 08.09.2021
         *
         * @param  base_ The base.
         */
        AdaptivePipelineResult(mref<base_type> base_) noexcept(_STD is_nothrow_move_constructible_v<base_type>);

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        ~AdaptivePipelineResult() noexcept = default;

    public:
        void operator>>(IN OUT ptr<value_type> ptr_) noexcept(_STD is_nothrow_move_constructible_v<value_type>) {

            using allocator_type = _STD allocator<value_type>;
            using allocator_traits = _STD allocator_traits<allocator_type>;

            allocator_type alloc {};
            allocator_traits::construct(alloc, ptr_, _STD forward<value_type>(this->get()));
        }

        void operator>>(IN OUT reference_type ref_) noexcept(_STD is_nothrow_move_assignable_v<value_type>) {
            ref_ = _STD move(this->get());
        }
    };

    template <>
    class AdaptivePipelineResult<void> :
        protected concurrent::future<void> { };
}
