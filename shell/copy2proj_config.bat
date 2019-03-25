set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

call loadpath.bat


rem -----------------------------------------------------------Copy DDRLocalServer Config DB to Project and  to Build Path-----------------------------------------------------------
rem Copy Server Db  to Project
rem xcopy /Y %LocalServerDBPath%\LocalServerDB.db %DDR_LocalServer_Project_Path% /s /i
rem xcopy /Y %RemoteServerDBPath%\RemoteServerDB.db %DDR_RemoteServer_Project_Path% /s /i
xcopy /Y %LocalServerDBPath%\LocalServerDB.db %DDR_LocalServer_Debug_Path% /s /i
xcopy /Y %RemoteServerDBPath%\RemoteServerDB.db %DDR_LocalServer_Debug_Path% /s /i

xcopy /Y %LocalServerDBPath%\LocalServerDB.db %DDR_LocalServer_Release_Path% /s /i
xcopy /Y %RemoteServerDBPath%\RemoteServerDB.db %DDR_LocalServer_Release_Path% /s /i

rem Copy Base Config to Project
rem xcopy /Y %BaseXmlPath%\*.xml %DDR_LocalClient_Project_Path%\Config  /i
rem xcopy /Y %BaseXmlPath%\*.xml %DDR_LocalServer_Project_Path%\Config  /i
rem xcopy /Y %BaseXmlPath%\*.xml %DDR_RemoteServer_Project_Path%\Config  /i
rem xcopy /Y %BaseXmlPath%\*.xml %DDR_BroadcastServer_Project_Path%\Config  /i
rem xcopy /Y %BaseXmlPath%\*.xml %DDR_VirtualService_Project_Path%\Config  /i


xcopy /Y %BaseXmlPath%\*.xml %DDR_LocalServer_Debug_Path%\Config  /i
xcopy /Y %BaseXmlPath%\*.xml %DDR_LocalServer_Release_Path%\Config  /i



rem Copy Each Self Config
rem xcopy /Y %LocalServerXmlPath% %DDR_LocalServer_Project_Path%\Config\LocalServer /s /i
rem xcopy /Y %ClientXmlPath% %DDR_LocalClient_Project_Path%\Config\Client /s /i
rem xcopy /Y %BroadcastServerXmlPath% %DDR_BroadcastServer_Project_Path%\Config\BroadcastServer /s /i
rem xcopy /Y %RemoteServerXmlPath% %DDR_RemoteServer_Project_Path%\Config\RemoteServer /s /i
rem xcopy /Y %VirtualServiceXmlPath% %DDR_VirtualService_Project_Path%\Config\VirtualService /s /i

rem Copy Server Project Config to Build
rem xcopy /Y %DDR_LocalClient_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
rem xcopy /Y %DDR_LocalClient_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i
rem xcopy /Y %DDR_LocalServer_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
rem xcopy /Y %DDR_LocalServer_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i
rem xcopy /Y %DDR_RemoteServer_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
rem xcopy /Y %DDR_RemoteServer_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i
rem xcopy /Y %DDR_BroadcastServer_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
rem xcopy /Y %DDR_BroadcastServer_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i
rem xcopy /Y %DDR_VirtualService_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
rem xcopy /Y %DDR_VirtualService_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i

xcopy /Y %LocalServerXmlPath% %DDR_LocalServer_Debug_Path%\Config\LocalServer /s /i
xcopy /Y %LocalServerXmlPath% %DDR_LocalServer_Release_Path%\Config\LocalServer /s /i
xcopy /Y %ClientXmlPath% %DDR_LocalServer_Debug_Path%\Config\Client /s /i
xcopy /Y %ClientXmlPath% %DDR_LocalServer_Release_Path%\Config\Client /s /i
xcopy /Y %BroadcastServerXmlPath% %DDR_LocalServer_Debug_Path%\Config\BroadcastServer /s /i
xcopy /Y %BroadcastServerXmlPath% %DDR_LocalServer_Release_Path%\Config\BroadcastServer /s /i
xcopy /Y %RemoteServerXmlPath% %DDR_LocalServer_Debug_Path%\Config\RemoteServer /s /i
xcopy /Y %RemoteServerXmlPath% %DDR_LocalServer_Release_Path%\Config\RemoteServer /s /i
xcopy /Y %VirtualServiceXmlPath% %DDR_LocalServer_Debug_Path%\Config\VirtualService /s /i
xcopy /Y %VirtualServiceXmlPath% %DDR_LocalServer_Release_Path%\Config\VirtualService /s /i



rem Copy DB to Build
rem xcopy /Y %DDR_LocalServer_Project_Path%\*.db %DDR_LocalServer_Debug_Path% /s /i
rem xcopy /Y %DDR_RemoteServer_Project_Path%\*.db %DDR_LocalServer_Debug_Path% /s /i

rem xcopy /Y %DDR_LocalServer_Project_Path%\*.db %DDR_LocalServer_Release_Path% /s /i
rem xcopy /Y %DDR_RemoteServer_Project_Path%\*.db %DDR_LocalServer_Release_Path% /s /i


rem -----------------------------------------------------------Copy DDRStreamRelayService Config to Project and to Build Path-----------------------------------------------------------


rem Copy Base Config to Project
rem xcopy /Y %BaseXmlPath%\*.xml %DDRStreamRelayService_Project_Path%\Config /i

xcopy /Y %BaseXmlPath%\*.xml %DDRStreamRelayService_Debug_Path%\Config /i
xcopy /Y %BaseXmlPath%\*.xml %DDRStreamRelayService_Release_Path%\Config /i

rem Copy Self Config
rem xcopy /Y %StreamRelayServiceXmlPath% %DDRStreamRelayService_Project_Path%\Config\StreamRelayService /s /i


rem Copy StreamRelay Server Project Config to Build
rem xcopy /Y %DDRStreamRelayService_Project_Path%\Config %DDRStreamRelayService_Debug_Path%\Config /s /i
rem xcopy /Y %DDRStreamRelayService_Project_Path%\Config %DDRStreamRelayService_Release_Path%\Config /s /i

xcopy /Y %StreamRelayServiceXmlPath% %DDRStreamRelayService_Debug_Path%\Config\StreamRelayService /s /i
xcopy /Y %StreamRelayServiceXmlPath% %DDRStreamRelayService_Release_Path%\Config\StreamRelayService /s /i


