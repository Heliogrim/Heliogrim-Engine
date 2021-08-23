#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define _DEBUG

//
struct InstanceData
{
	vec3 pos;
	float radius;
};

//
struct IndexedIndirectCommand
{
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	uint vertexOffset;
	uint firstInstance;
};

//
layout (binding = 0, std140) buffer Instances
{
	InstanceData instances[];
};

//
layout (binding = 1, std430) writeonly buffer IndirectDraws
{
	IndexedIndirectCommand indirectDraws[];
};

//
layout (binding = 2) uniform UBO
{
	mat4 projection;
	mat4 modelview;
	vec4 campos;
	vec4 planes[6];
} ubo;

#ifdef _DEBUG
//
layout (binding = 3) buffer StatsOut
{
	uint drawCount;
} stats;
#endif

/**
 * Check whether given position is between planes by radius
 *
 * @pos vec4 - position to check
 * @radius float - radius to check whether in planes
 *
 * return boolean - whether in frustum or not
 */
bool inFrustum(vec4 pos, float radius)
{
	for (int i = 0; i < 6; i ++)
	{
		if (dot(pos, ubo.planes[i]) + radius < 0.0)
			return false;
	}
	return true;
}

//
void main()
{
	uint idx = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x;

#ifdef _DEBUG
	if (idx == 0)
	{
		atomicExchange(stats.drawCount, 0);
	}
#endif

	vec4 pos = vec4(instances[idx].pos.xyz, 1.0);
	
	// Check Frustum
	if (inFrustum(pos, 1.0))
	{
		indirectDraws[idx].instanceCount = 1;

#ifdef _DEBUG
		atomicAdd(stats.drawCount, 1);
#endif
	} else
	{
		indirectDraws[idx].instanceCount = 0;
	}
}