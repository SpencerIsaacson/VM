@echo off
REM RELEASE BUILD
if "%~1"=="release" (
		cl -O2 -Os -Fehumidor.exe humidor.res client.c user32.lib SDL2.lib && if "%~2"=="run" humidor
) else (
REM DEBUG BUILD
		"compiler/tcc" -DDEBUG -DINCLUDE_EDITORS=1 client.c -o humidor.exe -luser32 Dependencies/SDL2.dll && if "%~1"=="run" humidor
)