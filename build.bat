@echo off
REM RELEASE BUILD
if "%~1"=="release" (
		cl -O2 -Os -Fehumidor.exe humidor.res client.c user32.lib SDL2.lib && if "%~2"=="run" humidor "%~3"
) else if "%~1"=="debug" (
		cl -Zi -Os -Fehumidor.exe humidor.res client.c user32.lib SDL2.lib && if "%~2"=="run" humidor "%~3"
) else (
REM DEV BUILD
		"compiler/tcc" -DDEBUG -DINCLUDE_EDITORS=1 client.c -o humidor.exe -luser32 Dependencies/SDL2.dll && if "%~1"=="run" humidor "%~2"
)