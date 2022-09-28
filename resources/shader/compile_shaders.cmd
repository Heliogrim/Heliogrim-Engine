@ECHO OFF

SET glslang="C:\VulkanSDK\1.2.162.1\Bin\glslangValidator.exe"

for %%a in (*.vs *.vert) do (
    %glslang% -V -S vert -v -o .\%%~na.vert.spv %%a
)
for %%a in (*.fs *.frag) do (
    %glslang% -V -S frag -v -o .\%%~na.frag.spv %%a
)
for %%a in (*.gs *.geom) do (
    %glslang% -V -S geom -v -o .\%%~na.geom.spv %%a
)
for %%a in (*.cps *.comp) do (
    %glslang% -V -S comp -v -o .\%%~na.comp.spv %%a
)

exit