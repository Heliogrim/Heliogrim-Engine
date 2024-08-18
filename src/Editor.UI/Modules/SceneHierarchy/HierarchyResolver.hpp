#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace hg::editor::ui {
	template <typename SourceType_, typename ResolveType_>
	class HierarchyResolver;
}

namespace hg::editor::ui {
	class HierarchyResolverBase {
	public:
		template <typename SourceType_, typename ResolveType_>
		friend class HierarchyResolver;

	public:
		using this_type = HierarchyResolverBase;

	private:
		HierarchyResolverBase() = default;

	public:
		virtual ~HierarchyResolverBase() = default;
	};

	template <typename SourceType_, typename ResolveType_ = SourceType_>
	class HierarchyResolver :
		public HierarchyResolverBase {
	public:
		using this_type = HierarchyResolver<SourceType_, ResolveType_>;

		using source_type = SourceType_;
		using resolve_type = ResolveType_;

	public:
		HierarchyResolver() :
			HierarchyResolverBase() {}

		~HierarchyResolver() override = default;

	public:
		void operator()(cref<SourceType_> source_, _Out_ ref<Vector<resolve_type>> resolved_) const;
	};

	template <>
	inline void HierarchyResolver<sptr<void>, sptr<void>>::operator()(
		cref<sptr<void>> source_,
		ref<Vector<resolve_type>> resolved_
	) const {}
}
