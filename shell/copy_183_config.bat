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

rem Copy Server Db and Config 
xcopy /Y %CurrentPath%..\tools\xlsx2xml\xml\Global_183.xml %CurrentPath%..\..\DDRLocalServer\DDR_LocalServer\Config\Global.xml /s /i
xcopy /Y %CurrentPath%..\tools\xlsx2xml\xml\Global_183.xml   %CurrentPath%..\..\DDRLocalServer\x64\Debug\Config\Global.xml /s /i
xcopy /Y %CurrentPath%..\tools\xlsx2xml\xml\Global_183.xml   %CurrentPath%..\..\DDRLocalServer\x64\Release\Config\Global.xml /s /i


xcopy /Y %CurrentPath%..\tools\xlsx2xml\xml\Global_183.xml %CurrentPath%..\..\DDRLocalServer\DDR_LocalClient\Config\Global.xml /s /i
xcopy /Y %CurrentPath%..\tools\xlsx2xml\xml\Global_183.xml   %CurrentPath%..\..\DDRLocalServer\x64\Debug\Config\Global.xml /s /i
xcopy /Y %CurrentPath%..\tools\xlsx2xml\xml\Global_183.xml   %CurrentPath%..\..\DDRLocalServer\x64\Release\Config\Global.xml /s /i


xcopy /Y %CurrentPath%..\tools\xlsx2xml\xml\Global_183.xml %CurrentPath%..\..\DDRStreamRelayService\DDRStreamRelayService\Config\Global.xml /s /i
xcopy /Y %CurrentPath%..\tools\xlsx2xml\xml\Global_183.xml   %CurrentPath%..\..\DDRStreamRelayService\x64\Debug\Config\Global.xml /s /i
xcopy /Y %CurrentPath%..\tools\xlsx2xml\xml\Global_183.xml   %CurrentPath%..\..\DDRStreamRelayService\x64\Release\Config\Global.xml /s /i

