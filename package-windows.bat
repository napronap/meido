@echo off

call %~dp0\vars.bat

set PACKAGE_CONFIG=%~1
if "%PACKAGE_CONFIG%"=="" set PACKAGE_CONFIG=Shipping

call "%RUNUAT_BAT%" BuildCookRun ^
 -project="%UPROJECT_PATH%" ^
 -target=%GAME_TARGET% ^
 -platform=Win64 ^
 -clientconfig=%PACKAGE_CONFIG% ^
 -build -cook -stage -pak -iostore -compressed -package -prereqs ^
 -archive -archivedirectory="%PACKAGE_DIR%" ^
 -nop4 -utf8output -unattended