
#include "CudaCompiler.h"
#include <reproc++/run.hpp>

bool CudaCompiler::hasFolderChanged (const std::string& folderPath, const std::string& jsonFilePath, const std::string& buildMode)
{
    nlohmann::json jsonFile;
    std::ifstream inFile (jsonFilePath);

    if (inFile.is_open())
    {
        inFile >> jsonFile;
        inFile.close();
    }
    else
    {
        jsonFile = nlohmann::json::object();
    }

    bool changed = false;

    // Check if build mode has changed
    if (jsonFile.find ("buildMode") == jsonFile.end() || jsonFile["buildMode"] != buildMode)
    {
        changed = true;
        jsonFile["buildMode"] = buildMode;
    }

    for (const auto& entry : std::filesystem::directory_iterator (folderPath))
    {
        auto path = entry.path();

        // Only consider *.cu files
        if (path.extension() == ".cu")
        {
            auto pathStr = path.string();
            auto lastWriteTime = std::filesystem::last_write_time (path);
            auto timeStr = lastWriteTime.time_since_epoch().count();

            if (jsonFile.find (pathStr) == jsonFile.end() || jsonFile[pathStr] != timeStr)
            {
                changed = true;
                jsonFile[pathStr] = timeStr;
            }
        }
    }

    std::ofstream outFile (jsonFilePath);
    if (outFile.is_open())
    {
        outFile << jsonFile.dump (4);
        outFile.close();
    }

    return changed;
}

void CudaCompiler::compile (const std::filesystem::path& resourceFolder, const std::filesystem::path& repoFolder)
{
    ScopedStopWatch sw (_FN_);

    std::string buildMode = "Release";
#ifndef NDEBUG
    buildMode = "Debug";
#endif

    verifyPath (resourceFolder);

    std::filesystem::path outputFolder = resourceFolder / "ptx";
    if (!std::filesystem::exists (outputFolder))
    {
        std::filesystem::create_directory (outputFolder);
    }

    verifyPath (repoFolder);

    std::filesystem::path cudaFolder = repoFolder / "sandbox" / "IBL" / "source" / "renderer" / "cuda";
    verifyPath (cudaFolder);

    // nothing to do if the cu files or build mode haven't been changed
    std::string jsonPath = outputFolder.string() + "/file_times.json";
    if (!hasFolderChanged (cudaFolder.string(), jsonPath, buildMode)) return;

    std::filesystem::path thirdPartyFolder = repoFolder / "thirdparty";
    verifyPath (thirdPartyFolder);

    std::filesystem::path shockerUtilFolder = repoFolder / "thirdparty/optiXUtil/src";
    verifyPath (shockerUtilFolder);

    std::filesystem::path eigenFolder = repoFolder / "thirdparty/linalg/eigen34";
    verifyPath (eigenFolder);

    // path to nvcc exe
    std::string exe = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.3/bin/nvcc.exe";

    std::string ext = ".cu";
    std::vector<std::filesystem::path> cuFiles = FileServices::findFilesWithExtension (cudaFolder, ext);

    for (const auto& f : cuFiles)
    {
        std::string fileName = f.filename().string();

        LOG (DBUG) << "Found: " << fileName;
        bool ptx = false;

        if (fileName.rfind ("copy", 0) == 0)
            ptx = true;

        // nvcc args
        std::vector<std::string> args;

        // path to nvcc exe
        args.push_back (exe);
        args.push_back (f.string());

        if (ptx)
            args.push_back ("--ptx");
        else
            args.push_back ("--optix-ir");
        args.push_back ("--extended-lambda");
        args.push_back ("--use_fast_math");
        args.push_back ("--cudart");
        args.push_back ("shared");
        args.push_back ("--std");
        args.push_back ("c++20");
        args.push_back ("-rdc");
        args.push_back ("true");
        args.push_back ("--expt-relaxed-constexpr");
        args.push_back ("--machine");
        args.push_back ("64");
        args.push_back ("--gpu-architecture");
        args.push_back ("sm_86"); // work for me, YMMV
        if (buildMode == "Debug")
        {
            args.push_back ("--debug");
            args.push_back ("--device-debug");
        }

        // NB if the ptx files are not being saved, first thing to do is check to make sure
        // this path is correct. It will be wrong if you have updated to a new version of vs2022
        args.push_back ("-ccbin");
        args.push_back ("C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.38.33130/bin/Hostx64/x64/");

        // OptiX 8 headers
        args.push_back ("--include-path");
        args.push_back ("C:/ProgramData/NVIDIA Corporation/OptiX SDK 8.0.0/include");

        // cuda 12.3 headers
        args.push_back ("--include-path");
        args.push_back ("C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.3/include");

        // OptixUtil
        args.push_back ("--include-path");
        args.push_back (shockerUtilFolder.generic_string());

        args.push_back ("--output-file");

        std::string outPath = ptx ? (outputFolder / f.stem()).string() + ".ptx" : (outputFolder / f.stem()).string() + ".optixir";

        LOG (DBUG) << outPath;
        args.push_back (outPath);

        int status = -1;
        std::error_code errCode;

        reproc::options options;
        options.redirect.parent = true;
        options.deadline = reproc::milliseconds (5000);

        std::tie (status, errCode) = reproc::run (args, options);

        if (errCode)
        {
            LOG (CRITICAL) << errCode.message();
        }
    }
}
void CudaCompiler::verifyPath (const std::filesystem::path& path)
{
    if (!std::filesystem::exists (path))
        throw std::runtime_error ("Invalid path: " + path.string());
}