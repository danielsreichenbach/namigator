@echo off
REM Build script for namigator using CMake presets on Windows

setlocal enabledelayedexpansion

REM Default values
set PRESET=windows-release
set INSTALL_PREFIX=C:\Program Files\namigator

REM Parse command line arguments
:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="--preset" (
    set PRESET=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--install-prefix" (
    set INSTALL_PREFIX=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--help" (
    echo Usage: %0 [OPTIONS]
    echo Options:
    echo   --preset PRESET         CMake preset to use ^(default: windows-release^)
    echo   --install-prefix PATH   Installation prefix ^(default: C:\Program Files\namigator^)
    echo   --help                 Show this help message
    echo.
    echo Available presets:
    cmake --list-presets 2>nul | findstr /r "\".*\""
    exit /b 0
)
echo Unknown option: %~1
exit /b 1
:end_parse

set BUILD_ROOT=%cd%
set BUILD_DIR=%BUILD_ROOT%\build\%PRESET%

echo Building namigator with preset: %PRESET%
echo Build directory: %BUILD_DIR%
echo Install prefix: %INSTALL_PREFIX%

REM CMake: configuration using preset
cmake --preset "%PRESET%" -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%"
if errorlevel 1 (
    echo Configuration failed!
    exit /b 1
)

REM CMake: build
cmake --build --preset "%PRESET%"
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

REM CMake: installation (may require administrator privileges)
echo.
echo Installing to %INSTALL_PREFIX%...
echo Note: This may require administrator privileges.
cmake --install "%BUILD_DIR%" --config Release
if errorlevel 1 (
    echo Installation failed! You may need to run this script as administrator.
    exit /b 1
)

echo.
echo Build completed successfully!
echo Namigator has been installed to: %INSTALL_PREFIX%

endlocal