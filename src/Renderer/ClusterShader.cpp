#include "ClusterShader.h"

#include "Scene/Scene.h"
#include "Renderer/Samplers.h"
#include <bx/math.h>
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>

bgfx::VertexDecl ClusterShader::ClusterVertex::decl;

ClusterShader::ClusterShader() :
    clusterSizesUniform(BGFX_INVALID_HANDLE),
    zNearFarVecUniform(BGFX_INVALID_HANDLE),
    clustersBuffer(BGFX_INVALID_HANDLE),
    lightIndicesBuffer(BGFX_INVALID_HANDLE),
    lightGridBuffer(BGFX_INVALID_HANDLE),
    atomicIndexBuffer(BGFX_INVALID_HANDLE),
    lightCount(0)
{
    static_assert(CLUSTERS_Z % CLUSTERS_Z_THREADS == 0, "number of cluster depth slices must be divisible by thread count z-dimension");
}

void ClusterShader::initialize()
{
    ClusterVertex::init();

    clusterSizesUniform = bgfx::createUniform("u_clusterSizes", bgfx::UniformType::Vec4);
    zNearFarVecUniform = bgfx::createUniform("u_zNearFarVec", bgfx::UniformType::Vec4);

    clustersBuffer     = bgfx::createDynamicVertexBuffer(CLUSTER_COUNT, ClusterVertex::decl,     BGFX_BUFFER_COMPUTE_READ_WRITE);
    lightIndicesBuffer = bgfx::createDynamicIndexBuffer (CLUSTER_COUNT * MAX_LIGHTS_PER_CLUSTER, BGFX_BUFFER_COMPUTE_READ_WRITE | BGFX_BUFFER_INDEX32);
    lightGridBuffer    = bgfx::createDynamicIndexBuffer (CLUSTER_COUNT * 4,                      BGFX_BUFFER_COMPUTE_READ_WRITE | BGFX_BUFFER_INDEX32);
    atomicIndexBuffer  = bgfx::createDynamicIndexBuffer (1,                                      BGFX_BUFFER_COMPUTE_READ_WRITE | BGFX_BUFFER_INDEX32);
}

void ClusterShader::shutdown()
{
    bgfx::destroy(clusterSizesUniform);
    bgfx::destroy(zNearFarVecUniform);

    bgfx::destroy(clustersBuffer);
    bgfx::destroy(lightIndicesBuffer);
    bgfx::destroy(lightGridBuffer);
    bgfx::destroy(atomicIndexBuffer);

    clusterSizesUniform = zNearFarVecUniform = BGFX_INVALID_HANDLE;
    clustersBuffer = BGFX_INVALID_HANDLE;
    lightIndicesBuffer = lightGridBuffer = atomicIndexBuffer = BGFX_INVALID_HANDLE;
}

void ClusterShader::setUniforms(const Scene* scene, uint16_t screenWidth, uint16_t screenHeight) const
{
    assert(scene != nullptr);

    float zNearFarVec[4] = { scene->camera.zNear, scene->camera.zFar };
    bgfx::setUniform(zNearFarVecUniform, zNearFarVec);

    // TODO why X?
    // use max instead? or use actual extent
    float clusterSizes[4] = { 0.0f, 0.0f, 0.0f, ceilf((float)screenWidth / CLUSTERS_X) };
    bgfx::setUniform(clusterSizesUniform, clusterSizes);
}

void ClusterShader::bindBuffers(bool lightingPass) const
{
    // binding ReadWrite in the fragment shader doesn't work
    bgfx::Access::Enum access = lightingPass ? bgfx::Access::Read : bgfx::Access::ReadWrite;
    if(!lightingPass)
    {
        bgfx::setBuffer(Samplers::CLUSTERS_CLUSTERS, clustersBuffer, access);
        bgfx::setBuffer(Samplers::CLUSTERS_ATOMICINDEX, atomicIndexBuffer, access);
    }
    bgfx::setBuffer(Samplers::CLUSTERS_LIGHTINDICES, lightIndicesBuffer, access);
    bgfx::setBuffer(Samplers::CLUSTERS_LIGHTGRID, lightGridBuffer, access);
}