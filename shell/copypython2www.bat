set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%



xcopy /Y %CurrentPath%\..\..\Shared\bin\x64\python\release %CurrentPath%\..\..\www\bin\x64\Release /s /i
xcopy /Y %CurrentPath%\..\..\Shared\bin\x64\python\debug %CurrentPath%\..\..\www\bin\x64\Debug /s /i


xcopy /Y %CurrentPath%\..\..\Shared\bin\x64\python\Lib %CurrentPath%\..\..\www\bin\x64\Release\Lib /s /i
xcopy /Y %CurrentPath%\..\..\Shared\bin\x64\python\Lib %CurrentPath%\..\..\www\bin\x64\Debug\Lib /s /i


xcopy /Y %CurrentPath%\..\..\Shared\script\Python %CurrentPath%\..\..\www\bin\x64\Release\Python /s /i
xcopy /Y %CurrentPath%\..\..\Shared\script\python %CurrentPath%\..\..\www\bin\x64\Debug\Python /s /i

cd %PrePath%