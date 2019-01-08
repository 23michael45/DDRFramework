set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%



xcopy /Y %CurrentPath%\..\..\DDRLocalServer\x64\Release\*.exe %CurrentPath%\..\..\www\bin\x64\Release\ /s /i
xcopy /Y %CurrentPath%\..\..\DDRStreamRelayService\x64\Release\*.exe %CurrentPath%\..\..\www\bin\x64\Release\ /s /i

xcopy /Y %CurrentPath%\..\..\DDRLocalServer\x64\Debug\*.exe %CurrentPath%\..\..\www\bin\x64\Debug\ /s /i
xcopy /Y %CurrentPath%\..\..\DDRStreamRelayService\x64\Debug\*.exe %CurrentPath%\..\..\www\bin\x64\Debug\ /s /i





cd %PrePath%