#pragma once

#include <Engine.Core/Module/RootModule.hpp>

#include "FnSets.hpp"

namespace hg::engine {
	class SerializationModule final :
		public core::RootModule,
		public serialization::FnSets {
	public:
		using this_type = SerializationModule;

	public:
		explicit SerializationModule() = default;

		SerializationModule(cref<this_type>) = delete;

		SerializationModule(mref<this_type>) = delete;

		~SerializationModule() = default;
	};
}
