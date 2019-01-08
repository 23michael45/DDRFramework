set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%


rem -----------------------------------------------------------Config DB Path------------------------------------------------------
set LocalServerDBPath=%CurrentPath%..\tools\sqlite3
set LocalServerXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\LocalServer

set BaseXmlPath=%CurrentPath%..\tools\xlsx2xml\xml
set ClientXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\Client
set StreamRelayServiceXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\StreamRelayService
set BroadcastServerXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\BroadcastServer
set MobileXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\MobileService



rem -----------------------------------------------------------Project Path---------------------------------------------------------
set DDR_LocalClient_Project_Path=%CurrentPath%\..\..\DDRLocalServer\DDR_LocalClient
set DDR_LocalServer_Project_Path=%CurrentPath%\..\..\DDRLocalServer\DDR_LocalServer
set DDR_BroadcastServer_Project_Path=%CurrentPath%\..\..\DDRLocalServer\DDR_BroadcastServer
set DDRStreamRelayService_Project_Path=%CurrentPath%\..\..\DDRStreamRelayService\DDRStreamRelayService



rem -----------------------------------------------------------Build Path-----------------------------------------------------------
set DDR_LocalServer_Debug_Path=%CurrentPath%\..\..\DDRLocalServer\x64\Debug
set DDR_LocalServer_Release_Path=%CurrentPath%\..\..\DDRLocalServer\x64\Release
set DDRStreamRelayService_Debug_Path=%CurrentPath%\..\..\DDRStreamRelayService\x64\Debug
set DDRStreamRelayService_Release_Path=%CurrentPath%\..\..\DDRStreamRelayService\x64\Release




rem -----------------------------------------------------------Copy DDRLocalServer Config DB to Project and  to Build Path-----------------------------------------------------------
rem Copy Server Db  to Project
xcopy /Y %LocalServerDBPath%\*.db %DDR_LocalServer_Project_Path% /s /i


rem Copy Base Config to Project
xcopy /Y %BaseXmlPath%\*.xml %DDR_LocalClient_Project_Path%\Config  /i
xcopy /Y %BaseXmlPath%\*.xml %DDR_LocalServer_Project_Path%\Config  /i
xcopy /Y %BaseXmlPath%\*.xml %DDR_BroadcastServer_Project_Path%\Config  /i

rem Copy Each Self Config
xcopy /Y %LocalServerXmlPath% %DDR_LocalServer_Project_Path%\Config\LocalServer /s /i
xcopy /Y %ClientXmlPath% %DDR_LocalClient_Project_Path%\Config\Client /s /i
xcopy /Y %BroadcastServerXmlPath% %DDR_BroadcastServer_Project_Path%\Config\BroadcastServer /s /i

rem Copy Server Project Config to Build
xcopy /Y %DDR_LocalClient_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
xcopy /Y %DDR_LocalClient_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i
xcopy /Y %DDR_LocalServer_Project_Path%\Config %DDR_LocalServer_Debug_Path%\Config /s /i
xcopy /Y %DDR_LocalServer_Project_Path%\Config %DDR_LocalServer_Release_Path%\Config /s /i
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


