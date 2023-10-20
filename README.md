# NanoBrain
Famework for experimenting with OptiX 8,0 using OptixUtility from shocker
https://github.com/shocker-0x15/OptiX_Utility

# Requires
Cuda 12.3
OptiX 8.0
and assumes that they have been installed in the default locations

# Compile OIIO
1. clone vcpkg into the thirdparty folder
https://github.com/microsoft/vcpkg

2. in the vcpkg folder, run bootstrap-vcpkg.bat to create vcpkg.exe

3. enter vcpkg folder in a terminal by right clicking on the vckpg folder icon and chosing "Open in Termainal" from the menu

4. in the terminal type:   vcpkg install openimageio:x64-windows-static 

# Compile ThirdParty libraries
1. goto thirdparty folder

2. run the generateVS2022.bat file to generate the thirdparty solution

3. in the newly created "thirdparty/builds" folder open the ThirdParty.sln found in the VisualStudio2022 folder

# Compile the sandbox samples
1 Goto NanoBrain root folder

2. run the generateVS2022.bat

3. in the newly created "builds" folder open the NanoBrain.sln found in the VisualStudio2022 folder
