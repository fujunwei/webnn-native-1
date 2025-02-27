@echo off

:: Copyright (C) 2018-2020 Intel Corporation
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::      http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.


@setlocal
SETLOCAL EnableDelayedExpansion
set "OPENVINO_VERSION=openvino_2021"
set "ROOT_DIR=%~dp0"
set "DISK_SYMBOL=%~d0"
::FOR /F "delims=\" %%i IN ("%ROOT_DIR%") DO set SAMPLES_TYPE=%%~nxi
echo %ROOT_DIR%
set "SOLUTION_DIR64=%ROOT_DIR%build"
::if "%InferenceEngine_DIR%"=="" set "InferenceEngine_DIR=%ROOT_DIR%\..\share"

set MSBUILD_BIN=
set VS_PATH=
set VS_VERSION=

if "%INTEL_OPENVINO_DIR%"=="" (
    if exist "%ProgramFiles(x86)%\Intel\%OPENVINO_VERSION%\bin\setupvars.bat" (
        call "%ProgramFiles(x86)%\Intel\%OPENVINO_VERSION%\bin\setupvars.bat"
    ) else (
        if exist "%ProgramFiles(x86)%\IntelSWTools\%OPENVINO_VERSION%\bin\setupvars.bat" (
            call "%ProgramFiles(x86)%\IntelSWTools\%OPENVINO_VERSION%\bin\setupvars.bat" 
      ) else (
         echo Failed to set the environment variables automatically    
         echo To fix, run the following command: ^<INSTALL_DIR^>\bin\setupvars.bat
         echo where INSTALL_DIR is the OpenVINO installation directory.
         GOTO errorHandling
      )
    )
) 

if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
   set "PLATFORM=x64"
) else (
   set "PLATFORM=Win32"
)

set VSWHERE="false"
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
   set VSWHERE="true"
   cd "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
   ::Make sure in C: directory to use tools in %ProgramFiles(x86)%
   C:
) else if exist "%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe" (
      set VSWHERE="true"
      cd "%ProgramFiles%\Microsoft Visual Studio\Installer"
      ::Make sure in C: directory to use tools in %ProgramFiles(x86)%
      C:
) else (
   echo "vswhere tool is not found"
)

if !VSWHERE! == "true" (
   if "!VS_VERSION!"=="" (
      echo Searching the latest Visual Studio...
      for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
         set VS_PATH=%%i
      )
   ) else (
      echo Searching Visual Studio !VS_VERSION!...
      for /f "usebackq tokens=*" %%i in (`vswhere -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
         set CUR_VS=%%i
         if not "!CUR_VS:%VS_VERSION%=!"=="!CUR_VS!" (
            set VS_PATH=!CUR_VS!
         )
      )
   )
   if exist "!VS_PATH!\MSBuild\14.0\Bin\MSBuild.exe" (
      set "MSBUILD_BIN=!VS_PATH!\MSBuild\14.0\Bin\MSBuild.exe"
   )
   if exist "!VS_PATH!\MSBuild\15.0\Bin\MSBuild.exe" (
      set "MSBUILD_BIN=!VS_PATH!\MSBuild\15.0\Bin\MSBuild.exe"
   )
   if exist "!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe" (
      set "MSBUILD_BIN=!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe"
   )
)

if "!MSBUILD_BIN!" == "" (
   if "!VS_VERSION!"=="2015" (
      if exist "C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe" (
         set "MSBUILD_BIN=C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe"
         set "MSBUILD_VERSION=14 2015"
      )
   ) else if "!VS_VERSION!"=="2017" (
      if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild.exe" (
         set "MSBUILD_BIN=C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild.exe"
         set "MSBUILD_VERSION=15 2017"
      ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin\MSBuild.exe" (
         set "MSBUILD_BIN=C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin\MSBuild.exe"
         set "MSBUILD_VERSION=15 2017"
      ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe" (
         set "MSBUILD_BIN=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"
         set "MSBUILD_VERSION=15 2017"
      )
   )
) else (
   if not "!MSBUILD_BIN:2019=!"=="!MSBUILD_BIN!" set "MSBUILD_VERSION=16 2019"
   if not "!MSBUILD_BIN:2017=!"=="!MSBUILD_BIN!" set "MSBUILD_VERSION=15 2017"
   if not "!MSBUILD_BIN:2015=!"=="!MSBUILD_BIN!" set "MSBUILD_VERSION=14 2015"
)

if "!MSBUILD_BIN!" == "" (
   echo Build tools for Microsoft Visual Studio !VS_VERSION! cannot be found. If you use Visual Studio 2017, please download and install build tools from https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2017
   GOTO errorHandling
)

echo Creating Visual Studio %MSBUILD_VERSION% %PLATFORM% files in %SOLUTION_DIR64%... && ^
:: back to current source directory
%DISK_SYMBOL%
cd "%ROOT_DIR%" && cmake -E make_directory "%SOLUTION_DIR64%" && cd "%SOLUTION_DIR64%" && cmake -G "Visual Studio !MSBUILD_VERSION!" -A %PLATFORM% "%ROOT_DIR%"

echo.
echo ###############^|^| Build ngraph_c_api using MS Visual Studio (MSBuild.exe) ^|^|###############
echo.
echo "!MSBUILD_BIN!" ngraph_c_api.sln /p:Configuration=Release
"!MSBUILD_BIN!" ngraph_c_api.sln /p:Configuration=Release
if ERRORLEVEL 1 GOTO errorHandling

:: copy the ngraph_c_api.dll to webnn native library directory.
set "WEBNN_NATIVE_LIB_PATH=%ROOT_DIR%\..\..\..\..\%1"
set "WEBNN_NATIVE_LIB_PATH=%WEBNN_NATIVE_LIB_PATH:/=\%"
xcopy "%ROOT_DIR%\build\intel64\Release\ngraph_c_api.dll" "%WEBNN_NATIVE_LIB_PATH%" /y
xcopy "%ROOT_DIR%\build\intel64\Release\ngraph_c_api.lib" "%WEBNN_NATIVE_LIB_PATH%" /y
mkdir "%WEBNN_NATIVE_LIB_PATH%\inference_engine\include\c_api
mkdir "%WEBNN_NATIVE_LIB_PATH%\inference_engine\lib\intel64\Release
xcopy "%INTEL_OPENVINO_DIR%\inference_engine\include\c_api" "%WEBNN_NATIVE_LIB_PATH%\inference_engine\include\c_api" /y
xcopy "%INTEL_OPENVINO_DIR%\inference_engine\lib\intel64\Release\inference_engine_c_api.lib" "%WEBNN_NATIVE_LIB_PATH%\inference_engine\lib\intel64\Release" /y

echo.
echo ###############^|^| Build ngraph_c_api succeeded ^|^|###############
echo.
goto :eof

:errorHandling
echo Error
