#pragma once

#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/ClassMetaBase.hpp>

#include "__fwd.hpp"
#include "../ResourceUsageFlag.hpp"

namespace hg::engine::resource {
	class __declspec(novtable) ResourceBase {
	public:
		friend class ManageGuard;

	public:
		using this_type = ResourceBase;

		using loaded_flag_type = u8;

	protected:
		ResourceBase() noexcept;

	public:
		virtual ~ResourceBase() = default;

	public:
		/**
		 * Get the current loading state of this resource
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @returns The loaded_flag_type value.
		 */
		[[nodiscard]] virtual loaded_flag_type loaded() const noexcept = 0;

		/**
		 * Check whether this is loaded
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @returns True if loaded, false if not.
		 */
		[[nodiscard]] bool isLoaded() const noexcept;

	public:
		/**
		 * Acquire this resource to use
		 *
		 * @author Julius
		 * @date 28.08.2021
		 *
		 * @param  flags_ (Optional) The flags.
		 *
		 * @returns A ManageGuard instance.
		 */
		[[nodiscard]] virtual ManageGuard acquire(
			const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
		) = 0;

		/**
		 * Attempts to acquire this resource to use
		 *
		 * @author Julius
		 * @date 28.08.2021
		 *
		 * @param  guard_ The guard.
		 * @param  flags_ (Optional) The flags.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] virtual bool try_acquire(
			_Out_ ref<ManageGuard> guard_,
			const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
		) noexcept = 0;

	protected:
		/**
		 * Releases this resource of use
		 *
		 * @details Should be guarded or only used with caution, cause most locks
		 *  or resource handlers won't save the acquirer signature.
		 *
		 * @author Julius
		 * @date 28.08.2021
		 */
		virtual void release(const ResourceUsageFlags flags_) = 0;

	protected:
		[[nodiscard]] virtual nmpt<void> value() const noexcept = 0;

	protected:
		/**/
		non_owning_rptr<const ClassMetaBase> _association;

	public:
		/**
		 * Get the stored association of this resource
		 *
		 * @note Previously defined as origin
		 *
		 * @returns A pointer to the stored association, otherwise nullptr
		 */
		[[nodiscard]] non_owning_rptr<const ClassMetaBase> getAssociation() const noexcept;

		/**
		 * Store a suitable association for this resource
		 *
		 * @note Previously defined as origin
		 *
		 * @author Julius
		 * @date 09.02.2023
		 *
		 * @param association_ A pointer used as association for this resource.
		 */
		void setAssociation(const non_owning_rptr<const ClassMetaBase> association_) noexcept;
	};
}
