@echo off
setlocal

:: --- Load Properties ---
set "WRAPPER_PROP=%~dp0wrapper\anvil-wrapper.properties"

if not exist "%WRAPPER_PROP%" (
    echo [Error] Cannot find %WRAPPER_PROP%
    exit /b 1
)

:: Read properties using PowerShell
for /f "usebackq tokens=*" %%a in (`powershell -Command "Get-Content '%WRAPPER_PROP%' | Select-String 'anvilVersion=' | ForEach-Object { $_.ToString().Split('=')[1].Trim() }"`) do set ANVIL_VERSION=%%a
for /f "usebackq tokens=*" %%a in (`powershell -Command "Get-Content '%WRAPPER_PROP%' | Select-String 'repoUrl=' | ForEach-Object { $_.ToString().Split('=')[1].Trim() }"`) do set REPO_URL=%%a

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
set "ANVIL_HOME=%~dp0.anvil\wrapper\%ANVIL_VERSION%"
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
if %errorlevel% neq 0 (
    echo [anvilw] Installation failed.
    del "%TEMP_ZIP%"
    exit /b 1
)
del "%TEMP_ZIP%"

if not exist "%ANVIL_BIN%" (
    echo [Error] %ANVIL_BIN% not found after installation.
    dir "%ANVIL_HOME%" /s
    exit /b 1
)

:exec
:: --- Execute ---
echo [anvilw] executing anvil: "%ANVIL_BIN%"
"%ANVIL_BIN%" %*
if %errorlevel% neq 0 (
    echo [anvilw] anvil exited with code %errorlevel%
    exit /b %errorlevel%
)