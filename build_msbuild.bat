@echo off

rem Скрипт для сборки библиотеки sqlite для Visual Studio 2008.

set bb.build.msbuild.exe=
for /D %%D in (%SYSTEMROOT%\Microsoft.NET\Framework\v4*) do set msbuild.exe=%%D\MSBuild.exe
if not defined msbuild.exe echo error: can't find MSBuild.exe & goto :eof
if not exist "%msbuild.exe%" echo error: %msbuild.exe%: not found & goto :eof
set VC_PROJECT_ENGINE_NOT_USING_REGISTRY_FOR_INIT=1

%msbuild.exe% %~dp0\sqlite.sln /t:Build /p:Configuration=Debug /p:Platform="Win32"
%msbuild.exe% %~dp0\sqlite.sln /t:Build /p:Configuration=Debug_Lib /p:Platform="Win32"
%msbuild.exe% %~dp0\sqlite.sln /t:Build /p:Configuration=Debug /p:Platform="x64"
%msbuild.exe% %~dp0\sqlite.sln /t:Build /p:Configuration=Debug_Lib /p:Platform="x64"
%msbuild.exe% %~dp0\sqlite.sln /t:Build /p:Configuration=Release /p:Platform="Win32"
%msbuild.exe% %~dp0\sqlite.sln /t:Build /p:Configuration=Release_Lib /p:Platform="Win32"
%msbuild.exe% %~dp0\sqlite.sln /t:Build /p:Configuration=Release /p:Platform="x64"
%msbuild.exe% %~dp0\sqlite.sln /t:Build /p:Configuration=Release_Lib /p:Platform="x64"
