#include "CommandBuffer.hpp"

#include <utility>
#include <Engine.Asserts/Todo.hpp>

#include "CommandPool.hpp"
#include "CommandQueue.hpp"
#include "../Buffer/IndexBufferView.hpp"
#include "../Buffer/SparseBufferView.hpp"
#include "../Buffer/VertexBufferView.hpp"
#include "../Framebuffer/Framebuffer.hpp"

using namespace hg::engine::gfx;
using namespace hg;

CommandBuffer::CommandBuffer(ptr<CommandPool> pool_, const vk::CommandBuffer& vkCmd_, const bool faf_) noexcept :
	_initialized(pool_ && vkCmd_),
	_recording(false),
	_valid(false),
	_faf(faf_),
	_root(true),
	_pool(pool_),
	_vkCmd(vkCmd_) {}

CommandBuffer::CommandBuffer(mref<CommandBuffer> other_) noexcept :
	_initialized(other_._initialized),
	_recording(other_._recording),
	_valid(other_._valid),
	_faf(other_._faf),
	_root(other_._root),
	_pipelineLayout(std::exchange(other_._pipelineLayout, nullptr)),
	_pipelineBindPoint(other_._pipelineBindPoint),
	_pool(std::exchange(other_._pool, nullptr)),
	_vkCmd(std::exchange(other_._vkCmd, nullptr)) {
	/**/
	other_._initialized = false;
	other_._recording = false;
	other_._valid = false;
	other_._faf = false;
	other_._root = true;
}

void CommandBuffer::begin(vk::CommandBufferInheritanceInfo* inheritanceInfo_) {
	assert(_initialized);

	vk::CommandBufferUsageFlags usage {
		_faf ?
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit :
		vk::CommandBufferUsageFlagBits::eSimultaneousUse
	};

	if (inheritanceInfo_ != nullptr) {
		usage |= vk::CommandBufferUsageFlagBits::eRenderPassContinue;
		_root = false;
	}

	const vk::CommandBufferBeginInfo info {
		usage,
		inheritanceInfo_
	};
	_vkCmd.begin(info);

	_recording = true;
}

void CommandBuffer::bindDescriptor(const Vector<vk::DescriptorSet>& descriptors_) {

	if (descriptors_.empty()) {
		return;
	}

	_vkCmd.bindDescriptorSets(
		_pipelineBindPoint,
		_pipelineLayout,
		0,
		static_cast<u32>(descriptors_.size()),
		descriptors_.data(),
		0,
		nullptr
	);
}

void CommandBuffer::bindDescriptor(const u32 id_, cref<vk::DescriptorSet> descriptor_) {

	_vkCmd.bindDescriptorSets(
		_pipelineBindPoint,
		_pipelineLayout,
		id_,
		1,
		&descriptor_,
		0,
		nullptr
	);
}

void CommandBuffer::bindIndexBuffer(const IndexBuffer& buffer_, u64 offset_) {
	_vkCmd.bindIndexBuffer(
		buffer_.buffer,
		offset_,
		vk::IndexType::eUint32
	);
}

void CommandBuffer::bindIndexBuffer(const ptr<const SparseBuffer> buffer_, u64 offset_) {
	_vkCmd.bindIndexBuffer(
		buffer_->vkBuffer(),
		offset_,
		vk::IndexType::eUint32
	);
}

void CommandBuffer::bindIndexBuffer(const ptr<const SparseBufferView> bufferView_) {
	_vkCmd.bindIndexBuffer(
		bufferView_->owner()->vkBuffer(),
		bufferView_->offset(),
		vk::IndexType::eUint32
	);
}

void CommandBuffer::bindIndexBuffer(cref<IndexBufferView> indexBufferView_) {
	_vkCmd.bindIndexBuffer(
		reinterpret_cast<VkBuffer>(indexBufferView_.buffer),
		indexBufferView_.offset,
		indexBufferView_.vertexIndexType == VertexIndexType::eU32 ? vk::IndexType::eUint32 : vk::IndexType::eUint16
	);
}

void CommandBuffer::bindVertexBuffer(const u32 binding_, cref<Buffer> buffer_, u64 offset_) {
	_vkCmd.bindVertexBuffers(
		binding_,
		1,
		&buffer_.buffer,
		&offset_
	);
}

void CommandBuffer::bindVertexBuffer(const u32 binding_, const VertexBuffer& buffer_, u64 offset_) {
	_vkCmd.bindVertexBuffers(
		binding_,
		1,
		&buffer_.buffer,
		&offset_
	);
}

void CommandBuffer::bindVertexBuffer(const u32 binding_, const ptr<const SparseBuffer> buffer_, u64 offset_) {
	_vkCmd.bindVertexBuffers(binding_, 1uL, &buffer_->vkBuffer(), &offset_);
}

void CommandBuffer::bindVertexBuffer(const u32 binding_, const ptr<const SparseBufferView> bufferView_) {
	const auto loff = bufferView_->offset();
	_vkCmd.bindVertexBuffers(binding_, 1uL, &bufferView_->owner()->vkBuffer(), &loff);
}

void CommandBuffer::bindVertexBuffer(const u32 index_, cref<VertexBufferView> vertexBufferView_) {
	vkCmdBindVertexBuffers(
		_vkCmd,
		index_,
		1uL,
		&reinterpret_cast<const VkBuffer&>(vertexBufferView_.buffer),
		&static_cast<const VkDeviceSize&>(vertexBufferView_.offset)
	);
}

void CommandBuffer::blit(vk::ImageBlit blit_, TextureBuffer& src_, TextureBuffer& dst_) {
	::hg::todo_panic();
}

void CommandBuffer::copyBuffer(const Buffer& src_, Buffer& dst_, const vk::BufferCopy& region_) {
	_vkCmd.copyBuffer(
		src_.buffer,
		dst_.buffer,
		1,
		&region_
	);
}

void CommandBuffer::copyBufferToImage(
	const Buffer& src_,
	TextureBuffer& dst_,
	const vk::BufferImageCopy& region_
) {
	_vkCmd.copyBufferToImage(
		src_.buffer,
		dst_.image(),
		dst_._vkLayout,
		1,
		&region_
	);
}

void CommandBuffer::copyImage(
	const TextureBuffer& src_,
	TextureBuffer& dst_,
	const vk::ImageCopy& region_
) {
	_vkCmd.copyImage(
		src_.image(),
		src_._vkLayout,
		dst_.image(),
		dst_._vkLayout,
		1,
		&region_
	);
}

void CommandBuffer::copyImageToBuffer(
	const TextureBuffer& src_,
	Buffer& dst_,
	const vk::BufferImageCopy& region_
) {
	_vkCmd.copyImageToBuffer(
		src_.image(),
		src_._vkLayout,
		dst_.buffer,
		1,
		&region_
	);
}

void CommandBuffer::draw(u32 instanceCount_, u32 instanceOffset_, u32 vertexCount_, u32 vertexOffset_) {
	_vkCmd.draw(vertexCount_, instanceCount_, vertexOffset_, instanceOffset_);
}

void CommandBuffer::drawIndexed(
	u32 instanceCount_,
	u32 instanceOffset_,
	u32 indexCount_,
	u32 indexOffset_,
	u32 vertexOffset_
) {
	_vkCmd.drawIndexed(indexCount_, instanceCount_, indexOffset_, vertexOffset_, instanceOffset_);
}

void CommandBuffer::drawIndirect(const TypeBuffer<vk::DrawIndirectCommand>& buffer_, u32 count_, u32 offset_) {
	_vkCmd.drawIndirect(buffer_.buffer, offset_, count_, sizeof(vk::DrawIndirectCommand));
}

void CommandBuffer::drawIndexedIndirect(
	const TypeBuffer<vk::DrawIndexedIndirectCommand>& buffer_,
	u32 count_,
	u32 offset_
) {
	_vkCmd.drawIndexedIndirect(buffer_.buffer, offset_, count_, sizeof(vk::DrawIndexedIndirectCommand));
}

void CommandBuffer::end() {
	_pipelineLayout = nullptr;
	_vkCmd.end();

	_recording = false;
	_valid = true;
}

void CommandBuffer::endRenderPass() {
	return _vkCmd.endRenderPass();
}

void CommandBuffer::reset() {
	assert(not _faf);

	_vkCmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

	_recording = false;
	_valid = false;
	_root = true;
}

ptr<CommandPool> CommandBuffer::pool() noexcept {
	return _pool;
}

hg::concurrent::UnfairSpinLock& CommandBuffer::lck() const {
	return _pool->lck();
}

void CommandBuffer::submitWait() {
	assert(_valid && _root);

	_pool->queue()->submitWait(*this);
	_valid = not _faf;
}

void CommandBuffer::release() {
	_pool->release(*this);
	_vkCmd = nullptr;

	_initialized = false;
	_recording = false;
	_valid = false;
	_faf = false;
	_root = false;
}

const vk::CommandBuffer& CommandBuffer::vkCommandBuffer() const noexcept {
	return _vkCmd;
}
