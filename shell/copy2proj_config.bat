set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

call loadpath.bat


rem -----------------------------------------------------------Copy DDRLocalServer Config DB to Project and  to Build Path-----------------------------------------------------------
rem Copy Server Db  to Project
xcopy /Y %LocalServerDBPath%\*.db %DDR_LocalServer_Project_Path% /s /i


rem Copy Base Config to Project
xcopy /Y %BaseXmlPath%\*.xml %DDR_LocalClient_Project_Path%\Config  /i
xcopy /Y %BaseXmlPath%\*.xml %DDR_LocalServer_Project_Path%\Config  /i
xcopy /Y %BaseXmlPath%\*.xml %DDR_RemoteServer_Project_Path%\Config  /i
xcopy /Y %BaseXmlPath%\*.xml %DDR_BroadcastServer_Project_Path%\Config  /i

rem Copy Each Self Config
xcopy /Y %LocalServerXmlPath% %DDR_LocalServer_Project_Path%\Config\LocalServer /s /i
xcopy /Y %ClientXmlPath% %DDR_LocalClient_Project_Path%\Config\Client /s /i
xcopy /Y %BroadcastServerXmlPath% %DDR_BroadcastServer_Project_Path%\Config\BroadcastServer /s /i
xcopy /Y %RemoteServerXmlPath% %DDR_RemoteServer_Project_Path%\Config\RemoteServer /s /i

rem Copy Server Project Config to Build
xcopy /Y %DDR_LocalClient_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
xcopy /Y %DDR_LocalClient_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i
xcopy /Y %DDR_LocalServer_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
xcopy /Y %DDR_LocalServer_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i
xcopy /Y %DDR_RemoteServer_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
xcopy /Y %DDR_RemoteServer_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i
xcopy /Y %DDR_BroadcastServer_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
xcopy /Y %DDR_BroadcastServer_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i


rem -----------------------------------------------------------Copy DDRStreamRelayService Config to Project and to Build Path-----------------------------------------------------------

rem Copy Base Config to Project
xcopy /Y %BaseXmlPath%\*.xml %DDRStreamRelayService_Project_Path%\Config /i
rem Copy Self Config
xcopy /Y %StreamRelayServiceXmlPath% %DDRStreamRelayService_Project_Path%\Config\StreamRelayService /s /i


rem Copy StreamRelay Server Project Config to Build
xcopy /Y %DDRStreamRelayService_Project_Path%\Config %DDRStreamRelayService_Debug_Path%\Config /s /i
xcopy /Y %DDRStreamRelayService_Project_Path%\Config %DDRStreamRelayService_Release_Path%\Config /s /i


