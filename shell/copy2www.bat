set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%



xcopy /Y %CurrentPath%\..\..\DDRLocalServer\x64\*.* %CurrentPath%\..\..\www\bin\x64 /s /i
xcopy /Y %CurrentPath%\..\..\DDRStreamRelayService\x64\*.* %CurrentPath%\..\..\www\bin\x64 /s /i

cd %PrePath%