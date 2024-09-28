#pragma once
#include "Meta/TypedMetaClass.hpp"

namespace hg {
	class Reflect {
	public:
		template <typename ActorType_>
		struct SubstitudeActorClass {
			static ptr<const MetaClass> Known() noexcept {
				return TypedMetaClass<ActorType_>::get();
			}

			static ptr<const MetaClass> Unknown() noexcept {
				return TypedMetaClass<ActorType_>::get();
			}
		};
	};
}
