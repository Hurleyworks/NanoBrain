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

// much taken from OptiX_Utility
// https://github.com/shocker-0x15/OptiX_Utility/blob/master/LICENSE.md

#include "../RenderContext.h"

// Forward declaration using shared_ptr
using PipelineHandlerRef = std::shared_ptr<class PipelineHandler>;

// Define the structure for the pipeline
template <typename EntryPointType>
struct Pipeline
{
    using Ptr = std::shared_ptr<Pipeline<EntryPointType>>;

    optixu::Pipeline optixPipeline;
    optixu::Module optixModule;
    optixu::Module emptyModule;
    optixu::Program emptyMissProgram;

    // Program entry points, programs and hit programs
    std::unordered_map<EntryPointType, optixu::Program> entryPoints;
    std::unordered_map<ProgramType, optixu::HitProgramGroup> hitPrograms;
    std::unordered_map<ProgramType, optixu::Program> programs;

    // Callable programs
    std::vector<optixu::CallableProgramGroup> callablePrograms;

    // Shader binding tables
    cudau::Buffer shaderBindingTable;
    cudau::Buffer sceneDependentSBT;

    // Set the entry point for the pipeline
    void setEntryPoint (EntryPointType et)
    {
        optixPipeline.setRayGenerationProgram (entryPoints.at (et));
    }
};

// Class to handle pipelines
class PipelineHandler
{
 public:
    // Create a shared_ptr instance of PipelineHandler
    static PipelineHandlerRef create (RenderContextPtr ctx) { return std::make_shared<PipelineHandler> (ctx); }

    // Alias for a database of pipelines
    using PipelineDB = std::unordered_map<EntryPointType, std::shared_ptr<Pipeline<EntryPointType>>>;

 public:
    PipelineHandler (RenderContextPtr ctx);
    ~PipelineHandler();

    // Retrieve or create a pipeline of a given type
    typename Pipeline<EntryPointType>::Ptr getPipeline (EntryPointType type)
    {
        auto it = pipelines.find (type);
        if (it != pipelines.end())
        {
            return it->second;
        }
        else
        {
            auto newPipeline = std::make_shared<Pipeline<EntryPointType>>();
            pipelines.emplace (type, newPipeline);
            return newPipeline;
        }
    }
    void setupPipeline (const PipelineData& data);
    void setupSBT (EntryPointType type);
    void setSceneDependentSBT(EntryPointType type);

 private:
    RenderContextPtr ctx = nullptr; // Render ctx reference
    PipelineDB pipelines;           // Database to store pipelines

};
