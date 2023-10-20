
local ROOT = "../"

	language "C++"

	defines{
		"JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED", "OIIO_STATIC_DEFINE", "__TBB_NO_IMPLICIT_LINKAGE", "_USE_MATH_DEFINES"
	}
	flags { "MultiProcessorCompile", "NoMinimalRebuild" }
	
	local VCPKG_DIR = "../../OpenSource/vcpkg/installed/x64-windows-static/"
	local CORE_DIR = ROOT .. "core/source/"
	local JAHLEY_DIR = ROOT .. "core/source/jahley/"
	local THIRD_PARTY_DIR = "../thirdparty/"
	local MODULE_DIR = "../modules/"
	
	local CUDA_INCLUDE_DIR = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.2/include"
	local CUDA_EXTRA_DIR = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.2/extras/cupti/include"
	local CUDA_LIB_DIR =  "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.2/lib/x64"
	
	local OPTIX_ROOT = "C:/ProgramData/NVIDIA Corporation"
	local OPTIX8_INCLUDE_DIR = OPTIX_ROOT .. "/OptiX SDK 8.0.0/include"
	
	includedirs
	{
		CORE_DIR,
		JAHLEY_DIR,
		MODULE_DIR,
		
		CUDA_INCLUDE_DIR,
		CUDA_EXTRA_DIR,
		OPTIX8_INCLUDE_DIR,
		
		-- for OIIO
		VCPKG_DIR,
		VCPKG_DIR .. "include",
		
		THIRD_PARTY_DIR,
		THIRD_PARTY_DIR .. "g3log/src",
		THIRD_PARTY_DIR .. "benchmark/include",
		THIRD_PARTY_DIR .. "json",
		THIRD_PARTY_DIR .. "dlib",
		THIRD_PARTY_DIR .. "libjpeg/src",
		THIRD_PARTY_DIR .. "stb_image",
		THIRD_PARTY_DIR .. "mnist_loader/src",
		THIRD_PARTY_DIR .. "glfw/include",
		THIRD_PARTY_DIR .. "nanogui/include",
		THIRD_PARTY_DIR .. "nanogui/ext/glad/include",
		THIRD_PARTY_DIR .. "nanogui/ext/nanovg/src",
		THIRD_PARTY_DIR .. "linalg/eigen34/Eigen",
		THIRD_PARTY_DIR .. "linalg/eigen34",
		THIRD_PARTY_DIR .. "JUCE/modules",
		THIRD_PARTY_DIR .. "nanothread/include/",
		THIRD_PARTY_DIR .. "taskflow/",
		THIRD_PARTY_DIR .. "binarytools/src",
		THIRD_PARTY_DIR .. "optiXUtil/src",
		THIRD_PARTY_DIR .. "pmp/src",
		THIRD_PARTY_DIR .. "reproc++",
		THIRD_PARTY_DIR .. "date/include/date",
		THIRD_PARTY_DIR .. "fast_obj/source/",
		THIRD_PARTY_DIR .. "geometry-central/include/",
		THIRD_PARTY_DIR .. "newtondynamics/sdk/**",
		THIRD_PARTY_DIR .. "newtondynamics/**",
		THIRD_PARTY_DIR .. "precompiled/include",
		THIRD_PARTY_DIR .. "precompiled/include/tbb",
		--THIRD_PARTY_DIR .. "precompiled/include/huse",
		THIRD_PARTY_DIR .. "precompiled/include/tiny-cuda-nn/include",
		THIRD_PARTY_DIR .. "precompiled/include/tiny-cuda-nn/include/tiny-cuda-nn/fmt/include",
	}
	
	targetdir (ROOT .. "builds/bin/" .. outputdir .. "/%{prj.name}")
	objdir (ROOT .. "builds/bin-int/" .. outputdir .. "/%{prj.name}")
	
	filter { "system:windows"}
		disablewarnings { 
			"5030", "4244", "4267", "4667", "4018", "4101", "4305", "4316", "4146", "4996", "4554",
		} 
		linkoptions { "-IGNORE:4099" } -- can't find debug file in release folder
		characterset ("MBCS")
		buildoptions { "/Zm250", "/bigobj", "/Zc:__cplusplus"}
		
		defines 
		{ 
			"WIN32", "_WINDOWS",
			--https://github.com/KjellKod/g3log/issues/337
			"_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING",
			"CHANGE_G3LOG_DEBUG_TO_DBUG",
			 "JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED", "NANOVG_GL3",
		}
		
	filter "configurations:Debug"
	
		postbuildcommands {
			
		}
		links 
		{ 
			"Core",
			"g3log",
			"benchmark",
			"dlib",
			--"libjpeg",
			"stb_image",
			"fmtd",
			"tiny-cuda-nn",
			"mnist_loader",
			"nanothread",
			"nanogui",
			"GLFW",
			"JUCE",
			"opengl32",
			"binarytools",
			"optiXUtil",
			"pmp",
			"geometry-central",
			"reproc++",
			"huse",
			"fast_obj",
			"rapidobj",
			"tbb",
			"newtondynamics",
			
			--cuda
			"cudart_static",
			"cuda",
			"nvrtc",
			"cublas",
			"curand",
			"cusolver",
			"cudart",
			"cudnn",
			
			--oiio
			"freetyped",
			"boost_thread-vc140-mt-gd",
			"boost_filesystem-vc140-mt-gd",
			"boost_system-vc140-mt-gd",
			"Iex-3_1_d",
			"IlmThread-3_1_d",
			"Imath-3_1_d",
			"jpeg",
			"turbojpeg",
			"tiffd",
			"zlibd",
			"lzma",
			"libpng16d",
			"OpenEXR-3_1_d",
			"OpenEXRCore-3_1_d",
			"OpenEXRUtil-3_1_d",
			"OpenImageIO_d",
			"OpenImageIO_Util_d",
			"brotlicommon-static",
			"brotlidec-static",
			"brotlienc-static",
			"bz2d",
		}
		defines { "DEBUG", "USE_DEBUG_EXCEPTIONS", "EIGEN_NO_DEBUG" }
		symbols "On"
		libdirs { THIRD_PARTY_DIR .. "builds/bin/" .. outputdir .. "/**",
				  ROOT .. "builds/bin/" .. outputdir .. "/**",
				  THIRD_PARTY_DIR .. "precompiled/bin/" .. outputdir .. "/**",
				  VCPKG_DIR .. "debug/lib",
				  CUDA_LIB_DIR
		}
		
	filter "configurations:Release"
	postbuildcommands {
			
		}
		links 
		{ 
			"Core",
			"g3log",
			"benchmark",
			"dlib",
			--"libjpeg",
			"stb_image",
			"fmt",
			"tiny-cuda-nn",
			"mnist_loader",
			"nanothread",
			"nanogui",
			"GLFW",
			"JUCE",
			"opengl32",
			"binarytools",
			"optiXUtil",
			"pmp",
			"geometry-central",
			"reproc++",
			"huse",
			"fast_obj",
			"rapidobj",
			"tbb",
			"newtondynamics",
			
			--cuda
			"cudart_static",
			"cuda",
			"nvrtc",
			"cublas",
			"curand",
			"cusolver",
			"cudart",
			"cudnn",
			
			 --for 0ii0
			 "freetype",
			"boost_thread-vc140-mt",
			"boost_filesystem-vc140-mt",
			"boost_system-vc140-mt",
			"Iex-3_1",
			"IlmThread-3_1",
			"Imath-3_1",
			"jpeg",
			"turbojpeg",
			"tiff",
			"zlib",
			"lzma",
			"libpng16",
			"OpenEXR-3_1",
			"OpenEXRCore-3_1",
			"OpenEXRUtil-3_1",
			"OpenImageIO",
			"OpenImageIO_Util",
			"brotlicommon-static",
			"brotlidec-static",
			"brotlienc-static",
			"bz2",
		}
		defines { "NDEBUG", "EIGEN_NO_DEBUG" }
		optimize "On"
		libdirs { THIRD_PARTY_DIR .. "builds/bin/" .. outputdir .. "/**",
				  ROOT .. "builds/bin/" .. outputdir .. "/**",
				  THIRD_PARTY_DIR .. "precompiled/bin/" .. outputdir .. "/**",
				  VCPKG_DIR .. "lib",
				  CUDA_LIB_DIR
		}
	
	  


	 		

