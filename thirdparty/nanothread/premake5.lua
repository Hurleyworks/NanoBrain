
project "nanothread"

	if _ACTION == "vs2019" then
		cppdialect "C++17"
		location ("../builds/VisualStudio2019/projects")
	end
	if _ACTION == "vs2022" then
		cppdialect "C++20"
		location ("../builds/VisualStudio2022/projects")
    end
    kind "SharedLib"
    language "C++"
  
    flags { "MultiProcessorCompile" }
	defines {"NANOTHREAD_BUILD"}
	targetdir ("../builds/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../builds/bin-int/" .. outputdir .. "/%{prj.name}")
	
	includedirs
	{
		"..",
		"include",
    }

	files
	{
		"include/**.h", 
		"src/**.h", 
		"src/**.cpp",  
    }
	
	
	filter "system:windows"
        staticruntime "On"
        systemversion "latest"
        disablewarnings { "4244", "4267", "4996", "4305", "4018", "4334", "4312", "4311", "4251", "4275", "4244" }
		defines { "_CRT_SECURE_NO_WARNINGS"}
		characterset "MBCS"

    filter "configurations:Debug"
        symbols "On"
                
    filter "configurations:Release"
        optimize "On"
	