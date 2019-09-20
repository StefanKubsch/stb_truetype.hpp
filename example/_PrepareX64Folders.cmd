RD .\x64\Release\Fonts /S /Q
RD .\x64\Release\Shader /S /Q

xcopy .\Fonts .\x64\Release\Fonts\ /S
xcopy .\Shader .\x64\Release\Shader\ /S
pause