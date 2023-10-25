
#pragma once

#include "mace_core/mace_core.h"

class CudaCompiler
{
 public:
    CudaCompiler() = default;
    ~CudaCompiler() = default;

    void compile (const std::filesystem::path& resourceFolder, const std::filesystem::path& repoFolder);
   
 private:
    
    void verifyPath (const std::filesystem::path& path);

}; // end class CudaCompiler