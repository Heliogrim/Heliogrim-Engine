#version 450

#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable

layout (local_size_x = 256) in;

/*
in uvec3 gl_NumWorkGroups;
in uvec3 gl_WorkGroupID;
in uvec3 gl_WorkGroupSize;
in uvec3 gl_LocalInvocationID;
in uvec3 gl_GlobalInvocationID;
in uint gl_LocalInvocationIndex;
*/

/*
struct CsfmHeader {
};
*/

struct CsfmBody {
	// 5468 Decisions => 683.5 Bytes => uint16_t[342]
	//uint16_t mte[342];
	uint32_t tme[171];
};

struct CsfmRow {
	//CsfmHeader header;
	CsfmBody body;
};

/*
*/
layout (set = 0, binding = 1) uniform usampler2D cmt;
/*
*/
layout (set = 0, binding = 2) buffer MaterialTranslateTable {
	//uint16_t mte[];
	uint32_t mte2[];
} mtt;

layout (set = 0, binding = 3) buffer CompactStreamFeedbackMasking {
	CsfmRow rows[];
} csfm;
/*
*/

//const uint16_t mip_bit_mask = uint16_t(0b1111'000000'000000);
const uint16_t mip_bit_mask = uint16_t(0xF000);
//const uint16_t tile_bit_mask = 0b0000'111111'111111;
const uint16_t tile_bit_mask = uint16_t(0x0FFF);

//const uint16_t off_bit_mask = 0b0000'000000'111111;
const uint16_t off_bit_mask = uint16_t(0x0007);
//const uint16_t dim_bit_mask = 0b0000'000000'111111;
const uint16_t dim_bit_mask = uint16_t(0x003F);

//
const uint16_t one = uint16_t(1);
const uint16_t two = uint16_t(2);

void markerFor(ivec2 pixel) {

	u32vec4 val = texelFetch(cmt, pixel, 0);

	/**
	 * Compute Mark Bitfield
	 */
	uint16_t matId = uint16_t(val.r >> 16);
	uint16_t tileId = uint16_t(val.r);

	uint16_t mip = (tileId & mip_bit_mask) >> 12;
	uint16_t tile = tileId & tile_bit_mask;

	// Get indirected material id
	uint16_t indMat = uint16_t(mtt.mte2[matId / 2]);

	// Get quad tree tile leaf
	uint16_t invMip = uint16_t(13) - mip;

	uint16_t qLeaf = (invMip > uint16_t(7)) ? invMip - uint16_t(8) : uint16_t(0);
	uint16_t offset = min(invMip, uint16_t(8));

	const uint16_t tdim = (
		one << (
			(
				invMip > uint16_t(7)
			) ? (
				invMip - uint16_t(7)
			) : (
				uint16_t(0)
			)
		)
	);

	if (qLeaf > 0) {
		for (uint16_t nd = uint16_t(0); nd < qLeaf; ++nd) {
			uint16_t pgc = two << nd;
			offset += (pgc * pgc /* Z-Collapse => * pgc */);
		}
	}

	// Flatten Dimensions (x,y,z) to linear offset :: `off = x + y * mx + z * my * mx`
	offset += (tile & dim_bit_mask); // X-Dimension :: Position [0..tdim)
	offset += ((tile >> 6) & dim_bit_mask) * tdim; // Y-Dimension :: Position [0..tdim)

	/**
	 * Store Bitfield
	 */
	csfm.rows[
		/* uint16_t(indMat) */
		//uint32_t(indMat)
		uint32_t(matId)
	].body.tme[
		offset /* (div) */ / 32
	] |= (1 << offset /* (modulo) */ % 32);

	/*
	// Debug
	csfm.rows[0].body.tme[0] = uint32_t(offset);// Low
	csfm.rows[0].body.tme[0] |= uint32_t(mip) << 16;// < High
	*/

}

void main() {

	//markerFor(ivec2(320, 460));

	uint32_t globalIndex = gl_LocalInvocationIndex + gl_WorkGroupID.x * gl_WorkGroupSize.x;

	markerFor(
		ivec2(
			globalIndex /* (modulo) */ % 1280,
			min(globalIndex /* (div) */ / 1280, 1279)
		)
	);
}