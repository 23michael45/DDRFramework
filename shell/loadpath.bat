set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%


rem -----------------------------------------------------------Config DB Path------------------------------------------------------
set LocalServerDBPath=%CurrentPath%..\tools\sqlite3
set RemoteServerDBPath=%CurrentPath%..\tools\sqlite3
set LocalServerXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\LocalServer
set RemoteServerXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\RemoteServer

set BaseXmlPath=%CurrentPath%..\tools\xlsx2xml\xml
set ClientXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\Client
set StreamRelayServiceXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\StreamRelayService
set BroadcastServerXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\BroadcastServer
set MobileXmlPath=%CurrentPath%..\tools\xlsx2xml\xml\MobileService



rem -----------------------------------------------------------Project Path---------------------------------------------------------
set DDR_LocalClient_Project_Path=%CurrentPath%\..\..\DDRLocalServer\DDR_LocalClient
set DDR_LocalServer_Project_Path=%CurrentPath%\..\..\DDRLocalServer\DDR_LocalServer
set DDR_BroadcastServer_Project_Path=%CurrentPath%\..\..\DDRLocalServer\DDR_BroadcastServer
set DDR_RemoteServer_Project_Path=%CurrentPath%\..\..\DDRLocalServer\DDR_RemoteServer
set DDRStreamRelayService_Project_Path=%CurrentPath%\..\..\DDRStreamRelayService\DDRStreamRelayService



rem -----------------------------------------------------------Build Path-----------------------------------------------------------
set DDR_LocalServer_Debug_Path=%CurrentPath%\..\..\DDRLocalServer\x64\Debug
set DDR_LocalServer_Release_Path=%CurrentPath%\..\..\DDRLocalServer\x64\Release
set DDRStreamRelayService_Debug_Path=%CurrentPath%\..\..\DDRStreamRelayService\x64\Debug
set DDRStreamRelayService_Release_Path=%CurrentPath%\..\..\DDRStreamRelayService\x64\Release



rem -----------------------------------------------------------WWW Path-----------------------------------------------------------
set WWW_Debug_Path=%CurrentPath%\..\..\www\bin\x64\Debug
set WWW_Release_Path=%CurrentPath%\..\..\www\bin\x64\Release

