#pragma once
#include <concepts>
#include <type_traits>
#include <utility>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/IsType.hpp>

#include "Symbol.hpp"

namespace hg::engine::render::graph {
	template <typename Validator_, typename Type_>
	concept HasValueValidation = requires(const Validator_& valid_, const nmpt<Type_> data_) {
		{ valid_.isValidObject(data_) } -> std::same_as<bool>;
	};
}

namespace hg::engine::render::graph {
	class SymbolizedResource {
	public:
		bool owned : 1/* Whether the resource is NOT owned by the context */ = false;
		bool transient : 1/* Currently unused */ = false;

		smr<const Symbol> symbol = nullptr;
		void* data = nullptr;

		// Warning: Temporary -> May be offloaded into a transition chaining layer
		Vector<void*> barriers;

		// TODO: Maybe we want to store the subject computed type identifier to extend dynamic type checking

	public:
		[[nodiscard]] bool empty() const noexcept {
			return data == nullptr;
		}

		[[nodiscard]] ptr<void> loadDirect() const noexcept {
			return data;
		}

		void storeDirect(const ptr<void> data_) noexcept {
			data = data_;
		}

		template <typename Type_, typename Validation_ = std::void_t<Type_>, typename Proj_ = std::identity> requires
			(not std::is_void_v<Type_>)
		[[nodiscard]] bool valid() const noexcept {

			if (empty()) {
				return std::is_same_v<Type_, ::std::nullptr_t>;
			}

			if constexpr (std::is_void_v<Validation_>) {
				return not std::is_same_v<Type_, ::std::nullptr_t>;
			}

			if constexpr (/* Type strengthening */not std::is_void_v<Validation_>) {

				if (not IsType<Validation_>(*symbol->description)) {
					return false;
				}

				if constexpr (HasValueValidation<Validation_, Type_>) {

					constexpr static Proj_ proj {};
					return static_cast<Validation_&>(*symbol->description).isValidObject(
						{ proj(*static_cast<Type_*>(data)) }
					);
				}

				return true;
			}

			std::unreachable();
		}

		template <typename Type_, typename Validation_ = std::void_t<Type_>, typename Proj_ = std::identity> requires
			std::is_move_constructible_v<Type_> && (not std::is_void_v<Type_>) && (not std::is_reference_v<Type_>)
		nmpt<Type_> create(mref<Type_> data_) noexcept(std::is_nothrow_move_constructible_v<Type_>) {

			assert(empty() && "Tried to create at already initialized symbolized resource.");

			if constexpr (not std::is_void_v<Validation_>) {

				if (not IsType<Validation_>(*symbol->description)) {
					// Warning: Rework
					return nullptr;
				}

				if constexpr (HasValueValidation<Validation_, Type_>) {

					constexpr static Proj_ proj {};
					if (static_cast<Validation_&>(*symbol->description).isValidObject({ proj(data_) })) {
						return nullptr;
					}
				}

			}

			data = new Type_(std::move(data_));
			return nmpt<Type_> { static_cast<Type_*>(data) };
		}

		template <typename Type_> requires std::is_void_v<Type_> && (not std::is_reference_v<Type_>)
		nmpt<Type_> create(mref<Type_> data_) noexcept(std::is_nothrow_move_constructible_v<Type_>) {

			assert(empty() && "Tried to create at already initialized symbolized resource.");

			data = new Type_(std::move(data_));
			return nmpt<Type_> { static_cast<Type_*>(data) };
		}

		template <typename Type_, typename Validation_ = std::void_t<Type_>> requires (not std::is_void_v<Type_>)
		[[nodiscard]] ref<Type_> load() const noexcept {

			assert(not empty() && "Tried to load from uninitialized symbolized resource.");

			if constexpr (not std::is_void_v<Validation_>) {
				if (not IsType<Validation_>(*symbol->description)) {
					return *static_cast<Type_*>(nullptr);
				}
			}

			return *static_cast<Type_*>(data);
		}

		template <typename Type_, typename Validation_ = std::void_t<Type_>, typename Proj_ = std::identity> requires
			(not std::is_void_v<Type_>)
		bool store(mref<Type_> data_) noexcept {

			assert(not empty() && "Tried to store to uninitialized symbolized resource.");

			if constexpr (not std::is_void_v<Validation_>) {

				if (not IsType<Validation_>(*symbol->description)) {
					return false;
				}

				if constexpr (HasValueValidation<Validation_, Type_>) {

					constexpr static Proj_ proj {};
					if (static_cast<Validation_&>(*symbol->description).isValidObject({ proj(data_) })) {
						return false;
					}
				}
			}

			*static_cast<Type_*>(data) = std::move(data_);
			return true;
		}

		template <typename Type_, typename Validation_ = std::void_t<Type_>> requires (not std::is_void_v<Type_>)
		bool destroy() noexcept(std::is_nothrow_destructible_v<Type_>) {

			if constexpr (not std::is_void_v<Validation_>) {
				if (not IsType<Validation_>(*symbol->description)) {
					return false;
				}
			}

			// TODO: Maybe we want to use reflect cast to strengthen the dynamic type checking

			if (not empty()) {
				delete static_cast<Type_*>(data);
				data = nullptr;
			}

			return empty();
		}
	};
}
