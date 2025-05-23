#pragma once

#include <Engine.Common/Sal.hpp>

#include "GlobalCacheCtrl.hpp"
#include "LocalResourceCache.hpp"

namespace hg::engine::gfx::cache {
	template <
		class SubjectType_,
		class SubResourceType_
	>
	struct LocalCacheCtrlSubject {
	public:
		using this_type = LocalCacheCtrlSubject<SubjectType_, SubResourceType_>;
		using subject_type = SubjectType_;
		using sub_resource_type = SubResourceType_;

	public:
		ptr<subject_type> subject;
		sub_resource_type subResource;
	};

	class LocalCacheCtrl final {
	public:
		friend class ::hg::engine::gfx::cache::LocalResourceCache;

	public:
		using this_type = LocalCacheCtrl;

		using texture_subject_type = LocalCacheCtrlSubject<TextureResource, AssocKey<TextureSubResource>>;
		using static_geometry_subject_type = LocalCacheCtrlSubject<StaticGeometryResource, StaticGeometrySubResource>;
		using material_subject_type = LocalCacheCtrlSubject<
			const void, std::pair<ptr<MaterialResource>, ptr<accel::AccelerationPipeline>>
		>;

	public:
		LocalCacheCtrl(mref<uptr<LocalResourceCache>> cache_) noexcept;

		LocalCacheCtrl(cref<this_type>) = delete;

		LocalCacheCtrl(mref<this_type>) noexcept = delete;

		~LocalCacheCtrl();

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type>) noexcept = delete;

	public:
		void tidy();

		void reset(const bool fully_ = false);

	private:
		uptr<LocalResourceCache> _cache;

	public:
		[[nodiscard]] const non_owning_rptr<const LocalResourceCache> cache() const noexcept;

		[[nodiscard]] const non_owning_rptr<LocalResourceCache> cache() noexcept;

	private:
		// TODO: Check whether we want to use as high-load hash-set
		Vector<texture_subject_type> _textures;
		Vector<static_geometry_subject_type> _staticGeometries;
		Vector<material_subject_type> _specMaterials;

	public:
		void markLoadedAsUsed(_In_ mref<smr<TextureResource>> resource_, _In_ mref<TextureSubResource> subResource_);

		void markLoadedAsUsed(
			_In_ mref<smr<TextureResource>> resource_,
			_In_ cref<AssocKey<TextureSubResource>> subResource_
		);

	public:
		void markAsUsed(const ptr<TextureResource> resource_, mref<TextureSubResource> subResource_);

		void markAsUsed(const ptr<TextureResource> resource_, cref<AssocKey<TextureSubResource>> subResource_);

		void markAsUsed(const ptr<StaticGeometryResource> resource_, mref<StaticGeometrySubResource> subResource_);

		void markAsUsed(
			_In_ const non_owning_rptr<const void> spec_,
			_In_ mref<smr<MaterialResource>> material_,
			_In_ mref<smr<accel::AccelerationPipeline>> accelerationPipeline_
		);
	};
}
