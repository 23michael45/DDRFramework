Xlsx2Xml.exe xlsx/ xml/

echo The current directory is %CD%
set CurrentPatrh=%CD%
xcopy /Y %CurrentPatrh%\xml\*.* %CurrentPatrh%\..\..\..\DDRLocalServer\DDR_LocalServer\Config /s /i