RD .\x64\Debug\Fonts /S /Q
RD .\x64\Release\Fonts /S /Q

xcopy .\Fonts .\x64\Debug\Fonts\ /S
xcopy .\Fonts .\x64\Release\Fonts\ /S
pause