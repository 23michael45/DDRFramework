set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

call loadpath.bat

xcopy /Y %DDR_LocalServer_Debug_Path%\Config %WWW_Debug_Path%\Config /s /i
xcopy /Y %DDRStreamRelayService_Debug_Path%\Config %WWW_Debug_Path%\Config /s /i


xcopy /Y %DDR_LocalServer_Release_Path%\Config %WWW_Release_Path%\Config /s /i
xcopy /Y %DDRStreamRelayService_Release_Path%\Config %WWW_Release_Path%\Config /s /i


xcopy /Y %DDR_LocalServer_Debug_Path%\*.db %WWW_Debug_Path% /s /i
xcopy /Y %DDR_LocalServer_Release_Path%\*.db %WWW_Release_Path% /s /i

cd %PrePath%