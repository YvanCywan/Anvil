# Anvil

Anvil is a modern, C++-based build system designed to be simple, extensible, and self-hosting. It allows you to define your build logic using C++ itself, providing the full power of the language for your build scripts.

## Project Structure

*   **`src/`**: Contains the source code for the Anvil build system itself.
    *   `src/main/`: The entry point and CLI command implementations (`build`, `clean`).
    *   `src/anvil/`: Core build logic, including the script compiler, driver, and toolchain management.
*   **`build.cpp`**: The build script for the Anvil project itself. This demonstrates how Anvil builds itself (dogfooding).
*   **`anvilw` / `anvilw.bat`**: The Anvil Wrapper. This script ensures you have the correct version of Anvil installed and runs it.
*   **`wrapper/`**: Configuration for the wrapper.

## Getting Started

### Prerequisites

*   A C++ compiler (Clang or GCC) to bootstrap the build script.
*   `unzip`, `curl` or `wget` (for the wrapper to download Anvil and dependencies).

### Building the Project

Anvil uses a wrapper script to bootstrap itself. You don't need to manually download or compile Anvil to get started.

1.  **Clone the repository:**
    ```bash
    git clone <repo_url>
    cd Anvil
    ```

2.  **Run the build:**
    Use the wrapper script to build the project.
    *   **Linux/macOS:**
        ```bash
        ./anvilw build
        ```
    *   **Windows:**
        ```cmd
        anvilw build
        ```

    The first time you run this, the wrapper will download the configured version of Anvil. Then, Anvil will compile the `build.cpp` script and execute it to build the project. Anvil will also automatically download `ninja` if it is not found.

3.  **Clean the build:**
    To remove build artifacts:
    ```bash
    ./anvilw clean
    ```

## Configuration

### Selecting a Compiler

You can configure which compiler Anvil uses for your project in your `build.cpp`:

```cpp
project.add_executable("my_app", [](anvil::CppApplication& app) {
    // Use GCC
    app.set_compiler(anvil::CompilerId::GCC);
    // OR Use Clang (default)
    app.set_compiler(anvil::CompilerId::Clang);
    
    // ... other settings
});
```

### Bootstrapping Compiler

By default, Anvil uses `clang++` to compile the `build.cpp` script itself. If you want to use `g++` for this bootstrap step, set the `ANVIL_SCRIPT_COMPILER` environment variable:

```bash
export ANVIL_SCRIPT_COMPILER=gcc
./anvilw build
```

## Developing Anvil

### Adding a New Command

Anvil's CLI is extensible. To add a new command:

1.  Create a new header file in `src/main/` (e.g., `my_command.hpp`).
2.  Inherit from the `anvil::Command` class.
3.  Implement `getName()`, `getDescription()`, and `execute()`.
4.  Register your command in `src/main/main.cpp`:
    ```cpp
    registry.registerCommand(std::make_unique<anvil::MyCommand>());
    ```

### Modifying the Build Logic

The core build logic resides in `src/anvil/`.
*   `script_compiler.hpp`: Handles compiling the user's `build.cpp`.
*   `driver.cpp`: The runtime that executes the compiled build script.
*   `toolchain.hpp`: Defines compiler interfaces (Clang, GCC).
*   `dependency_manager.hpp`: Handles downloading external tools like Ninja.

## Wrapper Configuration

The wrapper configuration is located in `wrapper/anvil-wrapper.properties`.
*   `anvilVersion`: The version of Anvil to use (e.g., `latest` or `1.0.0`).
*   `repoUrl`: The GitHub repository URL to download releases from.
