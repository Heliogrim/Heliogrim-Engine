#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "Geometry.hpp"

namespace hg::engine::assets {
	class StaticGeometry :
		public InheritMeta<StaticGeometry, Geometry> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static asset_type_id typeId { "StaticGeometry"_typeId };

	private:
	public:
		StaticGeometry();

	protected:
		StaticGeometry(mref<asset_guid> guid_);

	public:
		StaticGeometry(
			mref<asset_guid> guid_,
			mref<Vector<fs::Url>> sources_,
			cref<u64> vertexCount_,
			cref<u64> indexCount_
		);

	private:
		Vector<fs::Url> _sources;

	public:
		[[nodiscard]] cref<Vector<fs::Url>> sources() const noexcept;

		//private:
	public:
		u64 _vertexCount;
		u64 _indexCount;

		u64 _clusterCount;
		u64 _clusterDepth;

	public:
		[[nodiscard]] u64 getVertexCount() const;

		[[nodiscard]] u64 getIndexCount() const;

		[[nodiscard, deprecated]] u64 getFaceCount() const;

		[[nodiscard]] u32 getMaterialCount() const;

		[[nodiscard, deprecated]] bool hasUvCoords();

		[[nodiscard, deprecated]] bool hasUvm();

		[[nodiscard, deprecated]] bool hasLods();

		[[nodiscard, deprecated]] bool isStreamable();

		[[nodiscard]] math::Bounding getBoundary();
	};
}
