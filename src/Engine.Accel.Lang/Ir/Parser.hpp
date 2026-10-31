#pragma once

#include <expected>
#include <functional>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Diagnostic.hpp"
#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Returns the `.asl` source of the named module, or None if there is no such module.
	 */
	using ModuleResolver = std::function<Opt<String>(StringView name_)>;

	/**
	 * Parses one `.asl` module. `#include <name>` is module-level: the resolved module is parsed into the same
	 * result first, so its declarations precede the includer's. Includes must precede all declarations of a module;
	 * each module is included once per parse. Continues after errors and returns all diagnostics.
	 */
	[[nodiscard]] std::expected<Module, Diagnostics> parseModule(
		StringView name_,
		StringView source_,
		EntryKind entry_,
		ref<const ModuleResolver> includes_
	);
}
