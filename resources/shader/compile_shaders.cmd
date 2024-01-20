@ECHO OFF

SET glslang="C:\VulkanSDK\1.3.246.0\Bin\glslangValidator.exe"

for %%a in (*.vs *.vert) do (
    %glslang% -V -S vert --glsl-version 460 -v -o .\%%~na.vert.spv %%a
)
for %%a in (*.fs *.frag) do (
    %glslang% -V -S frag --glsl-version 460 -v -o .\%%~na.frag.spv %%a
)
for %%a in (*.gs *.geom) do (
    %glslang% -V -S geom --glsl-version 460 -v -o .\%%~na.geom.spv %%a
)
for %%a in (*.cps *.comp) do (
    %glslang% -V -S comp --glsl-version 460 -v -o .\%%~na.comp.spv %%a
)

exit