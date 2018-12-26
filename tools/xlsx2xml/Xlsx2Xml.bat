set PrePath=%CD%
echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

Xlsx2Xml.exe xlsx/ xml/

xcopy /Y %CurrentPath%\xml\*.* %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalServer\Config /s /i


xcopy /Y %CurrentPath%\xml\Client\*.* %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalClient\Config\Client /s /i
xcopy /Y %CurrentPath%\xml\Localization.* %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalClient\Config /s /i
xcopy /Y %CurrentPath%\xml\Global.* %CurrentPath%\..\..\..\DDRLocalServer\DDR_LocalClient\Config /s /i

xcopy /Y %CurrentPath%\xml\StreamRelayService\*.* %CurrentPath%\..\..\..\DDRStreamRelayService\DDRStreamRelayService\Config\StreamRelayService /s /i
xcopy /Y %CurrentPath%\xml\Localization.* %CurrentPath%\..\..\..\DDRStreamRelayService\DDRStreamRelayService\Config /s /i
xcopy /Y %CurrentPath%\xml\Global.* %CurrentPath%\..\..\..\DDRStreamRelayService\DDRStreamRelayService\Config /s /i

cd %PrePath%