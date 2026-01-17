@echo off
setlocal

:: --- Load Properties ---
set "WRAPPER_PROP=%~dp0wrapper\anvil-wrapper.properties"

if not exist "%WRAPPER_PROP%" (
    echo [Error] Cannot find %WRAPPER_PROP% 1>&2
    exit /b 1
)

:: Read properties using PowerShell
for /f "usebackq tokens=*" %%a in (`powershell -Command "Get-Content '%WRAPPER_PROP%' | Select-String 'anvilVersion=' | ForEach-Object { $_.ToString().Split('=')[1].Trim() }"`) do set ANVIL_VERSION=%%a
for /f "usebackq tokens=*" %%a in (`powershell -Command "Get-Content '%WRAPPER_PROP%' | Select-String 'repoUrl=' | ForEach-Object { $_.ToString().Split('=')[1].Trim() }"`) do set REPO_URL=%%a

if "%ANVIL_VERSION%"=="" ( echo [Error] anvilVersion not found 1>&2 & exit /b 1 )
if "%REPO_URL%"=="" ( echo [Error] repoUrl not found 1>&2 & exit /b 1 )

:: --- Determine Artifact & URL ---
set "ARTIFACT_NAME=anvil-windows-x64.zip"

if "%ANVIL_VERSION%"=="latest" (
    set "DOWNLOAD_URL=%REPO_URL%/releases/latest/download/%ARTIFACT_NAME%"
) else (
    set "DOWNLOAD_URL=%REPO_URL%/releases/download/v%ANVIL_VERSION%/%ARTIFACT_NAME%"
)

:: --- Define Cache Paths ---
set "ANVIL_HOME=%~dp0.anvil\wrapper\%ANVIL_VERSION%"
set "ANVIL_BIN=%ANVIL_HOME%\bin\anvil.exe"
set "TEMP_ZIP=%ANVIL_HOME%\anvil.zip"

:: --- Download & Install ---
if exist "%ANVIL_BIN%" goto exec

echo [anvilw] Downloading Anvil (%ANVIL_VERSION%)... 1>&2
if not exist "%ANVIL_HOME%" mkdir "%ANVIL_HOME%"

:: Download using PowerShell
powershell -Command "Invoke-WebRequest -Uri '%DOWNLOAD_URL%' -OutFile '%TEMP_ZIP%'"
if %errorlevel% neq 0 (
    echo [anvilw] Download failed. 1>&2
    exit /b 1
)

echo [anvilw] Installing... 1>&2
powershell -Command "Expand-Archive -Path '%TEMP_ZIP%' -DestinationPath '%ANVIL_HOME%' -Force"
if %errorlevel% neq 0 (
    echo [anvilw] Installation failed. 1>&2
    del "%TEMP_ZIP%"
    exit /b 1
)
del "%TEMP_ZIP%"

if not exist "%ANVIL_BIN%" (
    echo [Error] %ANVIL_BIN% not found after installation. 1>&2
    dir "%ANVIL_HOME%" /s 1>&2
    exit /b 1
)

:exec
:: --- Execute ---
echo [anvilw] executing anvil: "%ANVIL_BIN%" 1>&2

:: Attempt to patch missing DLLs from MinGW if available
where g++ >nul 2>&1
if %errorlevel% equ 0 (
    for /f "delims=" %%i in ('where g++') do set "GXX_BIN=%%~dpi"
)

if defined GXX_BIN (
    if not exist "%ANVIL_HOME%\bin\libstdc++-6.dll" if exist "%GXX_BIN%libstdc++-6.dll" copy "%GXX_BIN%libstdc++-6.dll" "%ANVIL_HOME%\bin\" >nul
    if not exist "%ANVIL_HOME%\bin\libgcc_s_seh-1.dll" if exist "%GXX_BIN%libgcc_s_seh-1.dll" copy "%GXX_BIN%libgcc_s_seh-1.dll" "%ANVIL_HOME%\bin\" >nul
    if not exist "%ANVIL_HOME%\bin\libwinpthread-1.dll" if exist "%GXX_BIN%libwinpthread-1.dll" copy "%GXX_BIN%libwinpthread-1.dll" "%ANVIL_HOME%\bin\" >nul
)

"%ANVIL_BIN%" %*
if %errorlevel% neq 0 (
    echo [anvilw] anvil exited with code %errorlevel% 1>&2
    exit /b %errorlevel%
)