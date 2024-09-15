#pragma once
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg::engine::storage {
	class macro_novtable IStorage :
		public InheritBase<IStorage>,
		public ArcFromThis<IStorage> {
	public:
		using this_type = IStorage;

	protected:
		IStorage() noexcept = default;

	public:
		constexpr ~IStorage() override = default;

	public:
		[[nodiscard]] virtual bool isPrimaryStorage() const noexcept = 0;

	public:
		[[nodiscard]] virtual bool isReadable() const noexcept = 0;

		[[nodiscard]] virtual bool isWriteable() const noexcept = 0;

		[[nodiscard]] virtual bool isRandomAccess() const noexcept = 0;

		[[nodiscard]] virtual bool isRandomReadable() const noexcept = 0;

		[[nodiscard]] virtual bool isRandomWritable() const noexcept = 0;
	};
}
