set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

Xlsx2Xml.exe xlsx/ xml/


cd %PrePath%