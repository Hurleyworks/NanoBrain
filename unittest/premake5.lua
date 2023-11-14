
-- https://github.com/JohannesMP/Premake-for-Beginners

workspace "UnitTest"
	architecture "x64"
	location ("builds")
	
if _ACTION == "vs2022" then
   location ("builds/VisualStudio2022")
end
if _ACTION == "vs2019" then
   location ("builds/VisualStudio2019")
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
	
	--add tests here
	include "tests/OIIO"
	include "tests/ShockerEigen"
	include "tests/Cereal"
	