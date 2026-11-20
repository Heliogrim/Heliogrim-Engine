#pragma once

#include "InputBindingList.hpp"
#include "../UiService.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
