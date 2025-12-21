#pragma once

#include <span>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Managed/AtomicRefCountedIntrusive.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Common/Memory/Pointer.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Heliogrim/Actor/Actor.hpp>

namespace hg::engine::core {
	class Level :
		public ArcFromThis<Level> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = Level;

	public:
		Level() noexcept = default;

		~Level();

	private:
		math::Bounding _bounding;

	public:
		[[nodiscard]] bool isRootLike() const noexcept;

		[[nodiscard]] cref<math::Bounding> getBounding() const noexcept;

		void setBounding(cref<math::Bounding> bounding_) noexcept;

	private:
		DenseSet<owner_ptr<Actor>> _actors;

	public:
		void addActor(_In_ mref<VolatileActor<>> actor_);

		[[nodiscard]] std::span<const owner_ptr<Actor>> getActors() const noexcept;

		[[nodiscard]] Opt<VolatileActor<>> removeActor(nmpt<Actor> actor_);
	};

	/**/

	[[nodiscard]] extern Arci<Level> make_root_like_level();
}