
-- https://github.com/JohannesMP/Premake-for-Beginners

-- Include the premake5 CUDA module
require('premake5-cuda')

workspace "NanoBrain"
	architecture "x64"
	location ("builds")

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
	vectorextensions "AVX2"
	filter "configurations:Debug"    defines { "DEBUG" }  symbols  "On"
    filter "configurations:Release"  defines { "NDEBUG" } optimize "On"
    
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
local ROOT = "../../"

-- the core
project "Core"
     kind "StaticLib"
    language "C++"
	defines { "NANOVG_GL3", "_ENABLE_EXTENDED_ALIGNED_STORAGE",
			"__TBB_NO_IMPLICIT_LINKAGE", "OIIO_STATIC_DEFINE", "CHANGE_G3LOG_DEBUG_TO_DBUG"}
	flags { "MultiProcessorCompile", "NoMinimalRebuild" }
	
	if _ACTION == "vs2019" then
		cppdialect "C++17"
		location "builds/VisualStudio2019/projects"
	end
	if _ACTION == "vs2022" then
		cppdialect "C++20"
		location ("builds/VisualStudio2022/projects")
	end
	
	pchheader "berserkpch.h"
    pchsource "core/source/berserkpch.cpp"
	
	targetdir ("builds/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("builds/bin-int/" .. outputdir .. "/%{prj.name}")

	local SOURCE_DIR = "core/source/"
	local BERSERKO_DIR = "core/source/jahley/"
	local MODULE_ROOT = "modules/"

    files
    { 
      SOURCE_DIR .. "**.h", 
      SOURCE_DIR .. "**.hpp", 
      SOURCE_DIR .. "**.c",
      SOURCE_DIR .. "**.cpp",
      SOURCE_DIR .. "**.tpp",
	  
	  MODULE_ROOT .. "**.h",  
      MODULE_ROOT .. "**.cpp",
	  MODULE_ROOT .. "**.cu",
    }
	
	local THIRD_PARTY_DIR = "thirdparty/"
	local VCPKG_DIR = "thirdparty/vcpkg/installed/x64-windows-static/"
	
	includedirs
	{
		SOURCE_DIR,
		BERSERKO_DIR,
		MODULE_ROOT,
		VCPKG_DIR,
		
		THIRD_PARTY_DIR,
		THIRD_PARTY_DIR .. "g3log/src",
		THIRD_PARTY_DIR .. "glfw/include",
		THIRD_PARTY_DIR .. "nanogui/include",
		THIRD_PARTY_DIR .. "nanogui/ext/glad/include",
		THIRD_PARTY_DIR .. "nanogui/ext/nanovg/src",
	
		VCPKG_DIR .. "include",
	}
	
	filter {} -- clear filter!
	
	 -- must go after files and includedirs
    filter { "files:modules/**/excludeFromBuild/**.cpp"}
		flags {"ExcludeFromBuild"}
	
	
	filter "system:windows"
        staticruntime "On"
        systemversion "latest"
		characterset ("MBCS")
        disablewarnings { 
			"5030", "4244", "4267", "4667", "4018", "4101", "4305", "4316", "4146", "4996",
		} 
		buildoptions { "/Zm250", "/bigobj","/Zc:__cplusplus",}
		
		defines 
		{ 
			"_CRT_SECURE_NO_WARNINGS",
			 "JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED", "NANOVG_GL3",
			--https://github.com/KjellKod/g3log/issues/337
			"_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING",
			"_USE_MATH_DEFINES", -- no need because OIIO defines it "NOMINMAX",
			
		}
		
		 -- setting up visual studio filters (basically virtual folders).
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
	
	filter "configurations:Debug"
        defines {"USE_DEBUG_EXCEPTIONS"}
    
	
	-- add projects here
	include "sandbox/HelloWorld"
	include "sandbox/HelloNanogui"
    include "sandbox/HelloOIIO"
	include "sandbox/GenerateNormals"