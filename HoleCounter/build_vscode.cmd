@echo off
setlocal

rem VS Code can inherit both LANG and lang, which breaks MSBuild's tool launcher.
set "LANG="
set "lang="

set "CMAKE_EXE=D:\Visual Studio2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

"%CMAKE_EXE%" -S "%~dp0." -B "%~dp0build" -G "Visual Studio 17 2022" -A x64 > "%~dp0build_vscode.log" 2>&1
if errorlevel 1 exit /b 1

"%CMAKE_EXE%" --build "%~dp0build" --config Debug --parallel >> "%~dp0build_vscode.log" 2>&1
exit /b %errorlevel%
