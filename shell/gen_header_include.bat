set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

rmdir /s /q %CurrentPath%\..\include\Shared\
mkdir  %CurrentPath%\..\include\Shared\


xcopy /Y %CurrentPath%\..\src\*.h %CurrentPath%\..\include\Shared\src /s /i
xcopy /Y %CurrentPath%\..\proto\*.h %CurrentPath%\..\include\Shared\proto /s /i
xcopy /Y %CurrentPath%\..\thirdparty\*.h %CurrentPath%\..\include\Shared\thirdparty /s /i





cd %PrePath%