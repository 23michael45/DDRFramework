set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

Xlsx2Xml.exe xlsx/ xml/

rem Copy Server Db and Config 
xcopy /Y %CurrentPath%\xml\*.* %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalServer\Config /s /i
xcopy /Y %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalServer\*.db   %CurrentPath%\..\..\..\DDRLocalServer\x64\Debug\
xcopy /Y %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalServer\*.db   %CurrentPath%\..\..\..\DDRLocalServer\x64\Release\
rem Copy Config To Bin
xcopy /Y %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalServer\Config\*.*   %CurrentPath%\..\..\..\DDRLocalServer\x64\Debug\Config\ /s /i
xcopy /Y %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalServer\Config\*.*   %CurrentPath%\..\..\..\DDRLocalServer\x64\Release\Config\ /s /i

rem Copy Client Config 
xcopy /Y %CurrentPath%\xml\Client\*.* %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalClient\Config\Client /s /i
xcopy /Y %CurrentPath%\xml\Localization.* %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalClient\Config /s /i
xcopy /Y %CurrentPath%\xml\Global.* %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalClient\Config /s /i
rem Copy Config To Bin
xcopy /Y %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalClient\Config\*.*   %CurrentPath%\..\..\..\DDRLocalServer\x64\Debug\Config\ /s /i
xcopy /Y %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalClient\Config\*.*   %CurrentPath%\..\..\..\DDRLocalServer\x64\Release\Config\ /s /i


rem Copy StreamRelay Config
xcopy /Y %CurrentPath%\xml\StreamRelayService\*.* %CurrentPath%\..\..\..\DDRStreamRelayService\DDRStreamRelayService\Config\StreamRelayService /s /i
xcopy /Y %CurrentPath%\xml\Localization.* %CurrentPath%\..\..\..\DDRStreamRelayService\DDRStreamRelayService\Config /s /i
xcopy /Y %CurrentPath%\xml\Global.* %CurrentPath%\..\..\..\DDRStreamRelayService\DDRStreamRelayService\Config /s /i
rem Copy Config To Bin
xcopy /Y %CurrentPath%\..\..\..\DDRStreamRelayService\DDRStreamRelayService\Config\*.*   %CurrentPath%\..\..\..\DDRStreamRelayService\x64\Debug\Config\ /s /i
xcopy /Y %CurrentPath%\..\..\..\DDRStreamRelayService\DDRStreamRelayService\Config\*.*   %CurrentPath%\..\..\..\DDRStreamRelayService\x64\Release\Config\ /s /i

cd %PrePath%