#include "FunctionTarget.hpp"

#include <Engine.Graphics/vkinc.hpp>
#include <Engine.Graphics/Buffer/Buffer.hpp>
#include <Engine.Graphics/Command/CommandBuffer.hpp>
#include <Engine.Graphics/Command/CommandPool.hpp>
#include <Engine.Graphics/Command/CommandQueue.hpp>
#include <Engine.Graphics/Memory/AllocationResult.hpp>
#include <Engine.Graphics/Memory/GlobalPooledAllocator.hpp>
#include <Engine.Graphics/Texture/TextureFactory.hpp>

import Engine.Common;

using namespace hg::test::gpu;
using namespace hg::engine::graphics;
using namespace hg;

namespace {
	[[nodiscard]] vk::ImageSubresourceRange colorRange();

	void transition(
		ref<const CommandBuffer> cmd_,
		ref<Texture> texture_,
		const vk::ImageLayout from_,
		const vk::ImageLayout to_,
		const vk::AccessFlags srcAccess_,
		const vk::AccessFlags dstAccess_,
		const vk::PipelineStageFlags srcStage_,
		const vk::PipelineStageFlags dstStage_
	);

	template <typename Record_>
	void submit(ref<const sptr<Device>> device_, Record_&& record_);

	// Destroys the readback buffer and frees its memory on every path
	struct Staging {
		Buffer buffer {};

		~Staging() {
			if (buffer.memory && buffer.memory->mapping) {
				buffer.unmap();
			}
			buffer.destroy();
		}
	};
}

FunctionTarget::FunctionTarget(ref<const sptr<Device>> device_, const math::uivec2 extent_) :
	_device(device_),
	_extent(extent_) {

	const auto* const factory = TextureFactory::get();
	auto texture = factory->build(
		{
			math::uivec3 { _extent.x, _extent.y, 1uL }, TextureFormat::eR32G32B32A32Sfloat, 1uL, TextureType::e2d,
			vk::ImageAspectFlagBits::eColor,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::SharingMode::eExclusive
		}
	);
	factory->buildView(texture, { .layers = { 0L, 1L }, .type = TextureType::e2d, .mipLevels = { 0L, 1L } });

	_texture = make_smr<Texture>(std::move(texture));
}

// ~Texture destroys the image and view
FunctionTarget::~FunctionTarget() = default;

void FunctionTarget::clear(const math::fvec4 value_) {

	submit(
		_device,
		[this, value_](ref<CommandBuffer> cmd_) {
			auto& texture = *_texture;
			transition(
				cmd_,
				texture,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal,
				vk::AccessFlags(),
				vk::AccessFlagBits::eTransferWrite,
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eTransfer
			);

			const auto color = vk::ClearColorValue { std::array<f32, 4> { value_.x, value_.y, value_.z, value_.w } };
			const auto range = colorRange();
			cmd_.vkCommandBuffer().clearColorImage(texture.buffer().image(), vk::ImageLayout::eTransferDstOptimal, &color, 1uL, &range);

			transition(
				cmd_,
				texture,
				vk::ImageLayout::eTransferDstOptimal,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::AccessFlagBits::eTransferWrite,
				vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eColorAttachmentOutput
			);
		}
	);
}

Raster FunctionTarget::read() {

	const auto texelCount = static_cast<size_t>(_extent.x) * _extent.y;
	const auto byteSize = static_cast<vk::DeviceSize>(texelCount * sizeof(math::fvec4));

	Staging staging {};
	auto& buffer = staging.buffer;
	buffer.device = _device->vkDevice();
	buffer.size = byteSize;
	buffer.usageFlags = vk::BufferUsageFlagBits::eTransferDst;
	buffer.buffer = _device->vkDevice().createBuffer(
		{ {}, byteSize, vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive }
	);

	const auto result = memory::allocate(
		*_device->allocator(),
		_device,
		vk::Buffer { buffer.buffer },
		MemoryProperty::eHostVisible | MemoryProperty::eHostCoherent,
		buffer.memory
	);
	if (result != memory::AllocationResult::eSuccess) {
		throw std::runtime_error("readback buffer allocation failed");
	}

	buffer.bind();

	/**/

	submit(
		_device,
		[this, &buffer](ref<CommandBuffer> cmd_) {
			auto& texture = *_texture;
			transition(
				cmd_,
				texture,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageLayout::eTransferSrcOptimal,
				vk::AccessFlagBits::eColorAttachmentWrite,
				vk::AccessFlagBits::eTransferRead,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eTransfer
			);

			// Zero row length and image height select tightly packed rows
			const auto region = vk::BufferImageCopy {
				0uLL, 0uL, 0uL, vk::ImageSubresourceLayers { vk::ImageAspectFlagBits::eColor, 0uL, 0uL, 1uL },
				vk::Offset3D { 0L, 0L, 0L }, vk::Extent3D { _extent.x, _extent.y, 1uL }
			};
			cmd_.copyImageToBuffer(texture.buffer(), buffer, region);

			const auto hostBarrier = vk::BufferMemoryBarrier {
				vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eHostRead, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				buffer.buffer, 0uLL, VK_WHOLE_SIZE
			};
			cmd_.vkCommandBuffer().pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eHost,
				vk::DependencyFlags(),
				0uL,
				nullptr,
				1uL,
				&hostBarrier,
				0uL,
				nullptr
			);

			transition(
				cmd_,
				texture,
				vk::ImageLayout::eTransferSrcOptimal,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::AccessFlagBits::eTransferRead,
				vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eColorAttachmentOutput
			);
		}
	);

	/**/

	Raster raster { .extent = _extent, .texels = Vector<math::fvec4>(texelCount) };

	buffer.map(byteSize);
	std::memcpy(raster.texels.data(), buffer.memory->mapping, byteSize);

	return raster;
}

smr<Texture> FunctionTarget::texture() const {
	return clone(_texture);
}

math::uivec2 FunctionTarget::extent() const {
	return _extent;
}

/**/

namespace {
	[[nodiscard]] vk::ImageSubresourceRange colorRange() {
		return vk::ImageSubresourceRange { vk::ImageAspectFlagBits::eColor, 0uL, 1uL, 0uL, 1uL };
	}

	void transition(
		ref<const CommandBuffer> cmd_,
		ref<Texture> texture_,
		const vk::ImageLayout from_,
		const vk::ImageLayout to_,
		const vk::AccessFlags srcAccess_,
		const vk::AccessFlags dstAccess_,
		const vk::PipelineStageFlags srcStage_,
		const vk::PipelineStageFlags dstStage_
	) {
		const auto barrier = vk::ImageMemoryBarrier {
			srcAccess_, dstAccess_, from_, to_, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture_.buffer().image(), colorRange()
		};
		cmd_.vkCommandBuffer().pipelineBarrier(srcStage_, dstStage_, vk::DependencyFlags(), 0uL, nullptr, 0uL, nullptr, 1uL, &barrier);
		texture_.buffer()._vkLayout = to_;
	}

	template <typename Record_>
	void submit(ref<const sptr<Device>> device_, Record_&& record_) {
		const auto queue = device_->graphicsQueue();
		// Frees the command buffer on every path, including throws from recording or submission
		struct Recording {
			CommandBuffer cmd;

			~Recording() {
				cmd.release();
			}
		} recording { queue->pool()->make() };

		recording.cmd.begin();
		std::forward<Record_>(record_)(recording.cmd);
		recording.cmd.end();
		queue->submitWait(recording.cmd);
	}
}
