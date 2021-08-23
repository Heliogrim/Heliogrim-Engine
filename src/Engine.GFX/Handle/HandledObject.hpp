#pragma once
#include <stdexcept>

namespace clockwork::__internal::gfx::handle {

	template <class HandlerType_>
	class HandledObject {
	public:

		using handler_type = HandlerType_*;

		/**
		 * \brief 
		 */
		virtual ~HandledObject() = default;

		/**
		 * \brief 
		 * \return 
		 */
		virtual bool ready() = 0;

		/**
		 * \brief 
		 * \return 
		 */
		inline handler_type handler() {
			return _handler;
		}

		/**
		 * \brief 
		 * \param handler_ 
		 */
		inline void handler(handler_type handler_) {
			if (_handler != nullptr) {
				throw _STD runtime_error("Handler is already registered.");
			}

			_handler = handler_;
		}

	private:
		handler_type _handler = nullptr;
	};
}
