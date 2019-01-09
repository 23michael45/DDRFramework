set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%


echo Compile Config
call %CurrentPath%\..\tools\xlsx2xml\Xlsx2Xml.bat

set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%


call loadpath.bat


rem copy to project
xcopy /Y %BaseXmlPath%\Global_183.xml %DDR_LocalClient_Project_Path%\Config\Global.xml /s /i
xcopy /Y %BaseXmlPath%\Global_183.xml %DDR_LocalServer_Project_Path%\Config\Global.xml /s /i
xcopy /Y %BaseXmlPath%\Global_183.xml %DDR_BroadcastServer_Project_Path%\Config\Global.xml /s /i
xcopy /Y %BaseXmlPath%\Global_183.xml %DDR_RemoteServer_Project_Path%\Config\Global.xml /s /i
xcopy /Y %BaseXmlPath%\Global_183.xml %DDRStreamRelayService_Project_Path%\Config\Global.xml /s /i


rem copy to build
xcopy /Y %BaseXmlPath%\Global_183.xml %DDR_LocalServer_Debug_Path%\Config\Global.xml /s /i
xcopy /Y %BaseXmlPath%\Global_183.xml %DDR_LocalServer_Release_Path%\Config\Global.xml /s /i
xcopy /Y %BaseXmlPath%\Global_183.xml %DDRStreamRelayService_Debug_Path%\Config\Global.xml /s /i
xcopy /Y %BaseXmlPath%\Global_183.xml %DDRStreamRelayService_Release_Path%\Config\Global.xml /s /i




