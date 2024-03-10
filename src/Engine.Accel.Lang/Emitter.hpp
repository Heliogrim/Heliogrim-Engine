#pragma once

#include <Engine.Common/Sal.hpp>

#include "Dialect.hpp"
#include "Language.hpp"
#include "Representation.hpp"

namespace hg::engine::accel::lang {
	class __declspec(novtable) Emitter {
	public:
		[[nodiscard]] virtual bool supportsDialect(Dialect dialect_) const noexcept = 0;

		virtual void operator()(cref<Representation> rep_, Dialect dialect_, _Out_ ref<Language> target_) const = 0;
	};
}
