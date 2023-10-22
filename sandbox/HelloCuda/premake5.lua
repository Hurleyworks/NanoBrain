local ROOT = "../../"

-- from https://github.com/theComputeKid/premake5-cuda

project  "HelloCuda"
	if _ACTION == "vs2019" then
		cppdialect "C++17"
		location (ROOT .. "builds/VisualStudio2019/projects")
    end
	
	if _ACTION == "vs2022" then
		cppdialect "C++20"
		location (ROOT .. "builds/VisualStudio2022/projects")
    end
	
	kind "ConsoleApp"

	local SOURCE_DIR = "source/*"
    files
    { 
      SOURCE_DIR .. "**.h", 
      SOURCE_DIR .. "**.hpp", 
      SOURCE_DIR .. "**.c",
      SOURCE_DIR .. "**.cpp",
    }
	
	includedirs
	{
	
	}
	
	  buildcustomizations "BuildCustomizations/CUDA 12.3"

  externalwarnings "Off" -- thrust gives a lot of warnings
  -- must be relative path from solution root
  cudaFiles { "../../sandbox/HelloCuda/source/**.cu" } -- files to be compiled into binaries
  cudaKeep "On" -- keep temporary output files
  cudaFastMath "On"
  cudaRelocatableCode "On"
  cudaVerbosePTXAS "Off"
  cudaCompilerOptions {"-arch=sm_86", "-t0"} 
  cudaMaxRegCount "32"

	
  filter "system:windows"
	staticruntime "On"
	systemversion "latest"
	defines {"_CRT_SECURE_NO_WARNINGS", "NOMINMAX"
	}
	
	disablewarnings { "5030" , "4305", "4316", "4267"}
	vpaths 
	{
	  ["Header Files/*"] = { 
		SOURCE_DIR .. "**.h", 
		SOURCE_DIR .. "**.hxx", 
		SOURCE_DIR .. "**.hpp",
	  },
	  ["Source Files/*"] = { 
		SOURCE_DIR .. "**.c", 
		SOURCE_DIR .. "**.cxx", 
		SOURCE_DIR .. "**.cpp",
	  },
	}
		
-- add settings common to all project
dofile("../../build_tools/common.lua")


