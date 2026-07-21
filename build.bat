@echo off
:: =========================================================
:: Ketsa Build System - Windows
:: =========================================================
:: Usage: build.bat [--release|--debug|--clean|--test]

setlocal enabledelayedexpansion

set BUILD_DIR=build
set CONFIG=Debug

:: Parse arguments
if "%1"=="--release" set CONFIG=Release
if "%1"=="--debug" set CONFIG=Debug
if "%1"=="--clean" goto :clean
if "%1"=="--test" goto :test

:: Check prerequisites
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [KETSA] ERROR: CMake not found. Install CMake 3.20+:
    echo         winget install Kitware.CMake
    exit /b 1
)

where ninja >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [KETSA] WARNING: Ninja not found. Falling back to MSBuild.
    set GENERATOR="Visual Studio 17 2022"
) else (
    set GENERATOR=Ninja
)

echo [KETSA] Building Ketsa (%CONFIG%)...
echo [KETSA] Generator: %GENERATOR%

if not exist %BUILD_DIR% mkdir %BUILD_DIR%

:: Detect Visual Studio
if "%GENERATOR%"=="Ninja" (
    for /f "tokens=*" %%i in ('"%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2^>nul') do set VS_PATH=%%i
    if defined VS_PATH (
        call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    ) else (
        echo [KETSA] WARNING: Visual Studio not detected. Build may fail.
    )
)

cd %BUILD_DIR%
cmake .. -G %GENERATOR% -DCMAKE_BUILD_TYPE=%CONFIG% >nul
if %ERRORLEVEL% neq 0 (
    cd ..
    echo [KETSA] ERROR: CMake configuration failed
    exit /b 1
)

cmake --build . --config %CONFIG% -- -j%NUMBER_OF_PROCESSORS%
if %ERRORLEVEL% neq 0 (
    cd ..
    echo [KETSA] ERROR: Build failed
    exit /b 1
)
cd ..

:: Copy binary
copy /Y "%BUILD_DIR%\ketsa.exe" "ketsa.exe" >nul 2>&1
echo [KETSA] Build successful: ketsa.exe
echo [KETSA] Run: ketsa --version
exit /b 0

:clean
echo [KETSA] Cleaning build...
if exist %BUILD_DIR% rmdir /S /Q %BUILD_DIR%
if exist "ketsa.exe" del "ketsa.exe"
echo [KETSA] Clean complete.
exit /b 0

:test
call :build
if %ERRORLEVEL% neq 0 exit /b 1
echo [KETSA] Running tests...
ketsa.exe --check examples\comprehensive.ketsa
if %ERRORLEVEL% neq 0 (
    echo [KETSA] Tests failed
    exit /b 1
)
echo [KETSA] All tests passed!
exit /b 0
