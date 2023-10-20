require("cuda-exported-variables")

if os.target() == "windows" then
    dofile("premake5-cuda-vs.lua")
end
