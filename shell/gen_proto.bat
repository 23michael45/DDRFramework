set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

echo Compile Proto
call %CurrentPath%\..\proto\compile_proto.bat

cd %PrePath%