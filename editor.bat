@echo off

call %~dp0\vars.bat

start "" "%UE56_EDITOR_EXE%" "%UPROJECT_PATH%" %*