
echo The current directory is %CD%
set CurrentPatrh=%CD%
cd %CurrentPatrh%
cd ..
cd ..

del /S *.log
del /S *.dmp