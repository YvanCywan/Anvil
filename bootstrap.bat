@echo off
echo [Anvil] Bootstrapping V0 on Windows...
if not exist bin mkdir bin

:: Compile main.cpp using MSVC (cl.exe)
:: /std:c++17 /EHsc (Exceptions) /Fe: output path
cl /nologo /std:c++17 /EHsc src\main\main.cpp /Fe:bin\anvil.exe

echo [Anvil] Bootstrap Complete.