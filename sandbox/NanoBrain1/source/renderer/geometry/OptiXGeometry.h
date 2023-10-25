#pragma once

#include "../RenderContext.h"
#include "../../scene/SpaceTime.h"

using Eigen::AlignedBox3f;

using OptiXGeometryRef = std::shared_ptr<class OptiXGeometry>;

class OptiXGeometry
{
 public:
    virtual ~OptiXGeometry()
    {
        geomInst.destroy();
        matIndexBuffer.finalize();
        gasData.gas.destroy();
        gasData.gasMem.finalize();
    }

    // Virtual functions requiring implementation
    virtual void createGeometry (RenderContextPtr ctx, SpaceTime& st, const MaterialInfo& info) = 0;

    // Some geometry can be parsed from a file
    virtual void fromFile (const std::filesystem::path& path) {}

    virtual void extractVertexPositions (MatrixXf& V) {}
    virtual void extractTriangleIndices (MatrixXu& F) {}

    // create a Geometry Acceleration Structure
    void createGAS (RenderContextPtr ctx, uint32_t numRayTypes)
    {
        gasData.gas = ctx->scene.createGeometryAccelerationStructure();
        gasData.gas.setConfiguration (
            optixu::ASTradeoff::PreferFastTrace,
            optixu::AllowUpdate::No,
            optixu::AllowCompaction::Yes);
        gasData.gas.setNumMaterialSets (1);
        gasData.gas.setNumRayTypes (0, numRayTypes);
        gasData.gas.addChild (geomInst);
        OptixAccelBufferSizes bufferSizes;
        gasData.gas.prepareForBuild (&bufferSizes);
        if (bufferSizes.tempSizeInBytes > ctx->asBuildScratchMem.sizeInBytes())
            ctx->asBuildScratchMem.resize (bufferSizes.tempSizeInBytes, 1, ctx->cuStr);

        gasData.gasMem.initialize (ctx->cuCtx, cudau::BufferType::Device, bufferSizes.outputSizeInBytes, 1);
    }

    GAS& getGAS() { return gasData; }

 protected:
    GAS gasData;
    std::filesystem::path filePath;
    optixu::GeometryInstance geomInst;
    cudau::TypedBuffer<uint8_t> matIndexBuffer;
};

template <typename VertexType, typename TriangleType, typename GeometryData>
class OptiXTriangleMesh : public OptiXGeometry
{
 public:
    static OptiXGeometryRef create()
    {
        return std::make_shared<OptiXTriangleMesh<VertexType, TriangleType, GeometryData>>();
    }

    ~OptiXTriangleMesh()
    {
        triangleBuffer.finalize();
        vertexBuffer.finalize();
    }

    void createGeometry (RenderContextPtr ctx, SpaceTime& st, const MaterialInfo& info) override;
    void fromFile (const std::filesystem::path& path) override { filePath = path; }
    void extractVertexPositions (MatrixXf& V) override;
    void extractTriangleIndices (MatrixXu& F) override;

 private:
    cudau::TypedBuffer<VertexType> vertexBuffer;
    cudau::TypedBuffer<TriangleType> triangleBuffer;
};
