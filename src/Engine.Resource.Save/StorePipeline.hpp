#pragma once

#include <concepts>
#include <type_traits>
#include <utility>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Meta/TypeList.hpp>

namespace hg::engine::resource {
	template </*typename Subject_,, */typename FinalFn_, typename... StageFn_>
	class StorePipeline :
		protected FinalFn_,
		protected StageFn_... {
	public:
		using this_type = StorePipeline</*Subject_, */FinalFn_, StageFn_...>;
		//using subject_type = Subject_;

	public:
		explicit StorePipeline(FinalFn_&& finalStage_, StageFn_&&... stages_) :
			FinalFn_(std::forward<FinalFn_>(finalStage_)),
			StageFn_(std::forward<StageFn_>(stages_))... {}

		constexpr virtual ~StorePipeline() noexcept = default;

	private:
		template <typename UniqueStageType_>
		[[nodiscard]] constexpr __restricted_ptr<const UniqueStageType_> getStage() const noexcept {
			return static_cast<const ptr<const UniqueStageType_>>(this);
		}

		template <typename Fn_, typename Next_, typename... Rest_>
		[[nodiscard]] decltype(auto) invoke(auto&&... args_) const {
			return (*getStage<Fn_>())(invoke<Next_, Rest_...>(std::forward<decltype(args_)>(args_)...));
		}

		template <typename Fn_>
		[[nodiscard]] decltype(auto) invoke(auto&&... args_) const {
			return (*getStage<Fn_>())(std::forward<decltype(args_)>(args_)...);
		}

	public:
		[[nodiscard]] decltype(auto) operator()(auto&&... args_) const
			requires std::is_invocable_v<last_type_of_t<FinalFn_, StageFn_...>, decltype(args_)...> {
			return invoke<FinalFn_, StageFn_...>(std::forward<decltype(args_)>(args_)...);
		}
	};

	/**/

	template <typename... StageFn_>
	StorePipeline(StageFn_&&... stages_) -> StorePipeline<StageFn_...>;
}
