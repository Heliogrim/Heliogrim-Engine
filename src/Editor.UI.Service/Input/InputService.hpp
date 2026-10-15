#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "InputBindingList.hpp"
#include "../UiService.hpp"

namespace hg::editor::ui::service {
	inline constexpr auto general_input_binding_list = "General"sv;

	class InputService :
		public UiService {
	public:
		using this_type = InputService;

	public:
		InputService();

		~InputService() override;

	private:
		DenseMap<String, UniquePtr<InputBindingList>> _bindingLists;

	public:
		[[nodiscard]] bool addInputBindings(const StringView key_, mref<UniquePtr<InputBindingList>> bindings_);

		[[nodiscard]] Opt<ref<const InputBindingList>> getInputBindings(ref<const String> key_) const noexcept;
	};
}
