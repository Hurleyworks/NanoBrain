require("build_tools/cuda-exported-variables")

if os.target() == "windows" then
    dofile("build_tools/premake5-cuda-vs.lua")
end
