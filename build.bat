@echo off
REM ─────────────────────────────────────────────────────────────────────────────
REM build.bat
REM Cleans, configures, and builds the Expense Tracker in Release mode.
REM Run this from the project root (where CMakeLists.txt lives).
REM
REM Edit QT_PATH below if your Qt install is in a different location.
REM ─────────────────────────────────────────────────────────────────────────────

set QT_PATH=C:\Qt\6.10.2\mingw_64

echo.
echo === Expense Tracker - Build ===
echo Qt path : %QT_PATH%
echo.

REM ── Guard: script must be run from the project root ───────────────────────
if not exist "%~dp0CMakeLists.txt" (
    echo ERROR: CMakeLists.txt not found next to build.bat.
    echo Run build.bat from the project root directory.
    pause
    exit /b 1
)

REM ── Guard: cmake must be on PATH ──────────────────────────────────────────
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: cmake not found in PATH.
    echo Install CMake from https://cmake.org/download/ and add it to PATH.
    pause
    exit /b 1
)

REM ── Guard: Qt install path must exist ─────────────────────────────────────
if not exist "%QT_PATH%" (
    echo ERROR: Qt not found at %QT_PATH%
    echo Edit the QT_PATH variable in this file to match your Qt install.
    pause
    exit /b 1
)

REM ── Guard: Qt CMake integration must be present ───────────────────────────
if not exist "%QT_PATH%\lib\cmake\Qt6\Qt6Config.cmake" (
    echo ERROR: Qt6 CMake files not found under:
    echo   %QT_PATH%\lib\cmake\Qt6
    echo Make sure QT_PATH points to the compiler-specific folder,
    echo e.g.  C:\Qt\6.x.x\mingw_64   or   C:\Qt\6.x.x\msvc2019_64
    pause
    exit /b 1
)

REM ── [1/5] Clean previous build ────────────────────────────────────────────
if exist build (
    echo [1/5] Cleaning previous build directory...
    rmdir /S /Q build
    if errorlevel 1 (
        echo ERROR: Could not remove the build directory.
        echo Close any files open from the build folder and try again.
        pause
        exit /b 1
    )
    echo       Done.
    echo.
)

REM ── [2/5] Configure ───────────────────────────────────────────────────────
echo [2/5] Configuring...
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QT_PATH%"
if errorlevel 1 (
    echo ERROR: CMake configure failed.
    echo Review the output above for details.
    pause
    exit /b 1
)

REM ── [3/5] Build ───────────────────────────────────────────────────────────
echo.
echo [3/5] Building...
cmake --build build --config Release --parallel
if errorlevel 1 (
    echo ERROR: Build failed.
    echo Review the compiler output above for details.
    pause
    exit /b 1
)

:: ---- Run windeployqt -------------------------------------------
echo [4/5] Running windeployqt (copies Qt DLLs + plugins)...

"%QT_PATH%\bin\windeployqt.exe" --release --no-translations "%~dp0build\cpp\ExpenseTracker.exe"

echo.
echo ============================================================
echo  Done! Your deployable app is in:
echo  %DEPLOY_DIR%
echo.
echo  You can run it from there or zip that folder to share.
echo ============================================================

REM ── [5/5] Done ────────────────────────────────────────────────────────────
echo.
echo [5/5] Done.
echo.
echo Build complete.
echo Executable: build\cpp\Release\ExpenseTracker.exe
pause
