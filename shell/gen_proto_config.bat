set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

echo Compile Proto
call %CurrentPath%\..\proto\compile_proto.bat

echo Compile Config
call %CurrentPath%\..\tools\xlsx2xml\Xlsx2Xml.bat

cd %PrePath%