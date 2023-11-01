-- Include the premake5 CUDA module
require('premake5-cuda')

workspace "Thirdparty"
	architecture "x64"
	location ("builds")
	vectorextensions "AVX2"

if _ACTION == "vs2019" then
   location ("builds/VisualStudio2019")
end

if _ACTION == "vs2022" then
   location ("builds/VisualStudio2022")
end
	configurations 
	{ 
		"Debug", 
        "Release",
    }
	
	filter "configurations:Debug"    defines { "DEBUG" }  symbols  "On"
    filter "configurations:Release"  defines { "NDEBUG" } optimize "On"
    
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- add projects
include "benchmark"
include "g3log"
include "optiXUtil"
include "glfw"
include "nanogui"
include "stb_image"
include "rapidobj"
include "newtondynamics"
include "reproc++"
include "cgltfReader"