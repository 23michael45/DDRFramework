set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%



xcopy /Y %CurrentPath%\..\src\*.h %CurrentPath%\..\include\Shared\src /s /i
xcopy /Y %CurrentPath%\..\thirdparty\*.h %CurrentPath%\..\include\Shared\thirdparty /s /i





cd %PrePath%