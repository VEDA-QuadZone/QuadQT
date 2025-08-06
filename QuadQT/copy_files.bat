@echo off
echo QuadQT DLL copy script

:: 가능한 빌드 디렉토리들을 순서대로 확인
set BUILD_DIRS=build\Desktop_Qt_6_9_1_MinGW_64_bit-Debug\debug build\Desktop_Qt_6_9_1_MinGW_64_bit-Release\release build\Desktop_Qt_6_9_1_MinGW_64_bit-Release2\release

set BUILD_DIR=
for %%d in (%BUILD_DIRS%) do (
    if exist "%%d" (
        set BUILD_DIR=%%d
        goto :found
    )
)

:found
if "%BUILD_DIR%"=="" (
    echo Error: No build directory found.
    echo Checked directories:
    for %%d in (%BUILD_DIRS%) do echo   %%d
    pause
    exit /b 1
)

echo Build directory found: %BUILD_DIR%

:: DLL 파일들이 이미 있는지 확인
if exist "%BUILD_DIR%\libopencv_core455.dll" (
    echo OpenCV DLL files already exist in build directory.
    echo Skipping copy operation.
) else (
    echo Copying OpenCV DLL files...
    if exist "dlls" (
        copy /Y "dlls\*.dll" "%BUILD_DIR%\"
        echo Copy completed!
    ) else (
        echo Warning: dlls directory not found.
    )
)

echo.
echo Build directory: %BUILD_DIR%
echo Ready to run QuadQT.exe
pause