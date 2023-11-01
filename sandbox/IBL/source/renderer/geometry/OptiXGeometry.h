/*
MIT License

Copyright (c) 2023 Steve Hurley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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
