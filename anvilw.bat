@echo off
setlocal

:: --- Load Properties ---
set "WRAPPER_PROP=wrapper\anvil-wrapper.properties"

if not exist "%WRAPPER_PROP%" (
    echo [Error] Cannot find %WRAPPER_PROP%
    exit /b 1
)

:: Read properties loops
for /f "tokens=2 delims==" %%a in ('findstr "anvilVersion=" %WRAPPER_PROP%') do set ANVIL_VERSION=%%a
for /f "tokens=2 delims==" %%a in ('findstr "repoUrl=" %WRAPPER_PROP%') do set REPO_URL=%%a

if "%ANVIL_VERSION%"=="" ( echo [Error] anvilVersion not found & exit /b 1 )
if "%REPO_URL%"=="" ( echo [Error] repoUrl not found & exit /b 1 )

:: --- Determine Artifact & URL ---
set "ARTIFACT_NAME=anvil-windows-x64.zip"

if "%ANVIL_VERSION%"=="latest" (
    set "DOWNLOAD_URL=%REPO_URL%/releases/latest/download/%ARTIFACT_NAME%"
) else (
    set "DOWNLOAD_URL=%REPO_URL%/releases/download/v%ANVIL_VERSION%/%ARTIFACT_NAME%"
)

:: --- Define Cache Paths ---
set "ANVIL_HOME=.anvil\wrapper\%ANVIL_VERSION%"
set "ANVIL_BIN=%ANVIL_HOME%\bin\anvil.exe"
set "TEMP_ZIP=%ANVIL_HOME%\anvil.zip"

:: --- Download & Install ---
if exist "%ANVIL_BIN%" goto exec

echo [anvilw] Downloading Anvil (%ANVIL_VERSION%)...
if not exist "%ANVIL_HOME%" mkdir "%ANVIL_HOME%"

:: Download using PowerShell
powershell -Command "Invoke-WebRequest -Uri '%DOWNLOAD_URL%' -OutFile '%TEMP_ZIP%'"
if %errorlevel% neq 0 (
    echo [anvilw] Download failed.
    exit /b 1
)

echo [anvilw] Installing...
powershell -Command "Expand-Archive -Path '%TEMP_ZIP%' -DestinationPath '%ANVIL_HOME%' -Force"
del "%TEMP_ZIP%"

:exec
:: --- Execute ---
"%ANVIL_BIN%" %*