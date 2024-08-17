#include "RenderCommandBuffer.hpp"

#include <utility>
#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Accel.Pipeline/ComputePipeline.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.GFX.Geometry/StaticMeshLayout.hpp>

#include "RenderCommandIterator.hpp"
#include "Commands/AttachResource.hpp"
#include "Commands/Begin.hpp"
#include "Commands/BeginAccelerationPass.hpp"
#include "Commands/BeginSubPass.hpp"
#include "Commands/BindIndexBuffer.hpp"
#include "Commands/BindPipeline.hpp"
#include "Commands/BindResourceTable.hpp"
#include "Commands/BindSkeletalMesh.hpp"
#include "Commands/BindStaticMesh.hpp"
#include "Commands/BindStorageBuffer.hpp"
#include "Commands/BindTexture.hpp"
#include "Commands/BindTextureSampler.hpp"
#include "Commands/BindUniformBuffer.hpp"
#include "Commands/BindVertexBuffer.hpp"
#include "Commands/DrawDispatch.hpp"
#include "Commands/DrawMesh.hpp"
#include "Commands/DrawSkeletalMesh.hpp"
#include "Commands/End.hpp"
#include "Commands/EndAccelerationPass.hpp"
#include "Commands/EndSubPass.hpp"
#include "Commands/Lambda.hpp"
#include "Commands/NextSubpass.hpp"

using namespace hg::engine::render::cmd;
using namespace hg;

static void link(ref<nmpt<RenderCommand>> prev_, mref<ptr<RenderCommand>> next_);

static void linkSafe(
	ref<nmpt<RenderCommand>> begin_,
	ref<nmpt<RenderCommand>> last_,
	mref<ptr<RenderCommand>> next_
);

/**/

RenderCommandBuffer::RenderCommandBuffer() noexcept :
	RenderCommandBufferBase(),
	RenderCommandAllocator(),
	_begin(nullptr),
	_last(nullptr) {}

RenderCommandBuffer::RenderCommandBuffer(mref<this_type> other_) noexcept :
	RenderCommandAllocator(std::move(other_)),
	_begin(std::exchange(other_._begin, nullptr)),
	_last(std::exchange(other_._last, nullptr)) {}

RenderCommandBuffer::~RenderCommandBuffer() noexcept {
	release();
}

cref<RenderCommandAllocator> RenderCommandBuffer::alloc() const noexcept {
	return *this;
}

ref<RenderCommandAllocator> RenderCommandBuffer::alloc() noexcept {
	return *this;
}

nmpt<RenderCommand> RenderCommandBuffer::root() const noexcept {
	return _begin;
}

void RenderCommandBuffer::release(ptr<RenderResourceTable> resourceTable_) {

	// TODO: Move stored resources into provided resource table if not null

	_last = nullptr;
	RenderCommandIterator iter { std::exchange(_begin, nmpt<RenderCommand> { nullptr }).get() };

	while (iter.valid()) {
		(iter++)->~RenderCommand();
	}

	alloc().release();
}

void RenderCommandBuffer::begin() noexcept {
	const auto result = alloc().allocateCommand<BeginRCmd>();

	if (_begin == nullptr) {
		_begin = result.value();
	}

	if (_last != nullptr) {
		_last->next = result.value();
	}

	_last = result.value();
}

void RenderCommandBuffer::beginAccelPass(mref<BeginAccelerationPassStruct> data_) noexcept {
	const auto result = alloc().allocateCommand<BeginAccelerationPassRCmd>(std::move(data_));
	link(_last, result.value());
}

void RenderCommandBuffer::beginSubPass(mref<BeginSubPassStruct> data_) noexcept {
	const auto result = alloc().allocateCommand<BeginSubPassRCmd>(std::move(data_));
	link(_last, result.value());
}

void RenderCommandBuffer::nextSubPass() noexcept {
	const auto result = alloc().allocateCommand<NextSubpassRCmd>();
	link(_last, result.value());
}

void RenderCommandBuffer::endSubPass() noexcept {
	const auto result = alloc().allocateCommand<EndSubPassRCmd>();
	link(_last, result.value());
}

void RenderCommandBuffer::endAccelPass() noexcept {
	const auto result = alloc().allocateCommand<EndAccelerationPassRCmd>();
	link(_last, result.value());
}

void RenderCommandBuffer::end() noexcept {
	auto result = alloc().allocateCommand<EndRCmd>();
	link(_last, result.value());
}

void RenderCommandBuffer::bindComputePipeline(mref<smr<const accel::ComputePipeline>> pipeline_) noexcept {
	const auto result = alloc().allocateCommand<BindPipelineRCmd>(
		pipeline_.into<const accel::AccelerationPipeline>(),
		BindPipelineFeatureSet::eCompute
	);
	link(_last, result.value());
}

void RenderCommandBuffer::bindGraphicsPipeline(mref<smr<const accel::GraphicsPipeline>> pipeline_) noexcept {
	const auto result = alloc().allocateCommand<BindPipelineRCmd>(
		pipeline_.into<const accel::AccelerationPipeline>(),
		BindPipelineFeatureSet::eGraphics
	);
	link(_last, result.value());
}

void RenderCommandBuffer::bindStaticMesh(const nmpt<const gfx::Mesh> mesh_) noexcept {
	const auto result = alloc().allocateCommand<BindStaticMeshRCmd>(std::move(mesh_));
	link(_last, result.value());
}

void RenderCommandBuffer::bindStaticMeshInstance(const nmpt<const gfx::MeshInstanceView> view_) noexcept {

	if constexpr (true) {

		const gfx::StorageBufferView* const sbv {};
		const auto result = alloc().allocateCommand<BindStorageBufferRCmd>(accel::lang::SymbolId {}, std::move(sbv));
		link(_last, result.value());

	} else {

		const gfx::VertexBufferView* const vbv {};
		const auto result = alloc().allocateCommand<BindVertexBufferRCmd>(gfx::VertexBufferView {});
		link(_last, result.value());
	}

}

void RenderCommandBuffer::bindSkeletalMesh(const nmpt<const gfx::Mesh> mesh_) noexcept {
	const auto result = alloc().allocateCommand<BindSkeletalMeshRCmd>(std::move(mesh_));
	link(_last, result.value());
}

void RenderCommandBuffer::bindSkeletalMeshInstance(
	const nmpt<const gfx::MeshInstanceView> meshView_,
	const nmpt<const gfx::SkeletalBoneView> boneView_
) noexcept {

	if constexpr (true) {

		const gfx::StorageBufferView* const isbv {};
		const gfx::StorageBufferView* const bsbv {};

		const auto instanceResult = alloc().allocateCommand<BindStorageBufferRCmd>(
			accel::lang::SymbolId {},
			std::move(isbv)
		);
		link(_last, instanceResult.value());

		const auto boneResult = alloc().allocateCommand<BindStorageBufferRCmd>(
			accel::lang::SymbolId {},
			std::move(bsbv)
		);
		link(_last, boneResult.value());

	} else {

		const gfx::VertexBufferView* const ivbv {};
		const gfx::StorageBufferView* const bsbv {};

		const auto instanceResult = alloc().allocateCommand<BindVertexBufferRCmd>(
			gfx::VertexBufferView {}
		);
		link(_last, instanceResult.value());

		const auto boneResult = alloc().allocateCommand<BindStorageBufferRCmd>(
			accel::lang::SymbolId {},
			std::move(bsbv)
		);
		link(_last, boneResult.value());
	}

}

void RenderCommandBuffer::bindIndexBuffer(const nmpt<const gfx::IndexBufferView> indexView_) noexcept {
	const auto result = alloc().allocateCommand<BindIndexBufferRCmd>(
		*indexView_
	);
	link(_last, result.value());
}

void RenderCommandBuffer::bindVertexBuffer(const nmpt<const gfx::VertexBufferView> vertexView_) noexcept {
	const auto result = alloc().allocateCommand<BindVertexBufferRCmd>(
		*vertexView_
	);
	link(_last, result.value());
}

void RenderCommandBuffer::bindStorage(
	mref<accel::lang::SymbolId> symbolId_,
	const nmpt<const gfx::StorageBufferView> storageView_
) noexcept {
	const auto result = alloc().allocateCommand<BindStorageBufferRCmd>(
		std::move(symbolId_),
		std::move(storageView_)
	);
	link(_last, result.value());
}

void RenderCommandBuffer::bindTexture(
	mref<accel::lang::SymbolId> symbolId_,
	mref<nmpt<const gfx::TextureLikeObject>> texture_
) noexcept {

	assert(texture_);

	const auto result = alloc().allocateCommand<BindTextureRCmd>(
		std::move(symbolId_),
		std::move(texture_)
	);
	link(_last, result.value());
}

void RenderCommandBuffer::bindTextureSampler(
	mref<accel::lang::SymbolId> symbolId_,
	mref<nmpt<const gfx::TextureSampler>> sampler_
) noexcept {

	assert(sampler_);

	const auto result = alloc().allocateCommand<BindTextureSamplerRCmd>(
		std::move(symbolId_),
		std::move(sampler_)
	);
	link(_last, result.value());
}

void RenderCommandBuffer::bindUniform(
	mref<accel::lang::SymbolId> symbolId_,
	const nmpt<const gfx::UniformBufferView> uniformView_
) noexcept {
	const auto result = alloc().allocateCommand<BindUniformBufferRCmd>(
		std::move(symbolId_),
		std::move(uniformView_)
	);
	link(_last, result.value());
}

void RenderCommandBuffer::bind(mref<smr<engine::render::ResourceTable>> table_) noexcept {
	const auto result = alloc().allocateCommand<BindResourceTableRCmd>(
		std::move(table_)
	);
	link(_last, result.value());
}

void RenderCommandBuffer::drawMesh(
	const nmpt<const gfx::MeshDescription> meshDescription_,
	u32 instanceCount_,
	u32 instanceOffset_,
	u32 primitiveCount_,
	u32 primitiveOffset_
) noexcept {
	const auto result = alloc().allocateCommand<DrawMeshRCmd>(
		meshDescription_,
		instanceCount_,
		instanceOffset_,
		primitiveCount_,
		primitiveOffset_,
		false
	);
	link(_last, result.value());
}

void RenderCommandBuffer::drawMeshIdx(
	const nmpt<const gfx::MeshDescription> meshDescription_,
	u32 instanceCount_,
	u32 instanceOffset_,
	u32 primitiveCount_,
	u32 primitiveOffset_
) noexcept {
	const auto result = alloc().allocateCommand<DrawMeshRCmd>(
		meshDescription_,
		instanceCount_,
		instanceOffset_,
		primitiveCount_,
		primitiveOffset_,
		true
	);
	link(_last, result.value());
}

void RenderCommandBuffer::drawSkeletalMesh(
	u32 instanceCount_,
	u32 instanceOffset_,
	u32 primitiveCount_,
	u32 primitiveOffset_
) noexcept {
	const auto result = alloc().allocateCommand<DrawSkeletalMeshRCmd>(
		instanceCount_,
		instanceOffset_,
		primitiveCount_,
		primitiveOffset_,
		false
	);
	link(_last, result.value());
}

void RenderCommandBuffer::drawSkeletalMeshIdx(
	u32 instanceCount_,
	u32 instanceOffset_,
	u32 primitiveCount_,
	u32 primitiveOffset_
) noexcept {
	const auto result = alloc().allocateCommand<DrawSkeletalMeshRCmd>(
		instanceCount_,
		instanceOffset_,
		primitiveCount_,
		primitiveOffset_,
		true
	);
	link(_last, result.value());
}

void RenderCommandBuffer::drawStaticMesh(
	u32 instanceCount_,
	u32 instanceOffset_,
	u32 primitiveCount_,
	u32 primitiveOffset_
) noexcept {
	this_type::drawMesh(
		gfx::GetStaticMeshLayout().get(),
		instanceCount_,
		instanceOffset_,
		primitiveCount_,
		primitiveOffset_
	);
}

void RenderCommandBuffer::drawStaticMeshIdx(
	u32 instanceCount_,
	u32 instanceOffset_,
	u32 primitiveCount_,
	u32 primitiveOffset_
) noexcept {
	this_type::drawMeshIdx(
		gfx::GetStaticMeshLayout().get(),
		instanceCount_,
		instanceOffset_,
		primitiveCount_,
		primitiveOffset_
	);
}

void RenderCommandBuffer::drawDispatch(
	u32 instanceCount_,
	u32 instanceOffset_,
	u32 vertexCount_,
	u32 vertexOffset_
) noexcept {
	const auto result = alloc().allocateCommand<DrawDispatchRCmd>(
		instanceCount_,
		instanceOffset_,
		vertexCount_,
		vertexOffset_,
		false
	);
	link(_last, result.value());
}

void RenderCommandBuffer::lambda(mref<std::function<void(ref<accel::AccelCommandBuffer>)>> lambda_) noexcept {
	const auto result = alloc().allocateCommand<LambdaRCmd>(
		std::move(lambda_)
	);
	link(_last, result.value());
}

void RenderCommandBuffer::attach(mref<smr<void>> obj_) noexcept {}

/**/

void link(ref<nmpt<RenderCommand>> prev_, mref<ptr<RenderCommand>> next_) {
	prev_->next = next_;
	prev_ = next_;
}

void linkSafe(
	ref<nmpt<RenderCommand>> begin_,
	ref<nmpt<RenderCommand>> last_,
	mref<ptr<RenderCommand>> next_
) {
	if (begin_ == nullptr) {
		begin_ = next_;
	}

	if (last_ != nullptr) {
		last_->next = next_;
	}

	last_ = next_;
}
