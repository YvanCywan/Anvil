@echo off
echo [Anvil] Bootstrapping V0 on Windows...

if not exist bin mkdir bin

cl /nologo /std:c++17 /EHsc /I src src\main\main.cpp /Fe:bin\anvil.exe

if %errorlevel% neq 0 (
    echo [Anvil] Bootstrap FAILED.
    exit /b %errorlevel%
)

echo [Anvil] Bootstrap Complete.