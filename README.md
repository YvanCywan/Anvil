# Anvil

Anvil is a modern, C++-based build system designed for simplicity, extensibility, and self-sufficiency. It empowers you to define your build logic using C++ itself, giving you the full power of the language for your build scripts without requiring complex external languages or tools.

At its core, Anvil is a single executable that compiles and runs a `build.cpp` file in your project's root. To make this process seamless, it comes with a lightweight wrapper (`anvilw`) that automatically downloads the correct Anvil version for your platform.

## Features

*   **C++ Build Scripts**: Define your entire build process in standard C++. No need to learn another language like Gradle, Make, or CMake.
*   **Zero Setup**: The `anvilw` wrapper automatically downloads the Anvil executable and its dependencies. Users of your project don't need to install anything other than a C++ compiler.
*   **Self-Contained**: The Anvil executable includes all necessary logic to compile your build script and manage dependencies.
*   **Automatic Toolchain Management**: Anvil automatically downloads and manages `ninja` for high-performance builds.
*   **Cross-Platform**: Works on Linux, macOS, and Windows.
*   **Extensible**: Easily add new commands and functionality to the core Anvil executable.

## How It Works

Anvil's bootstrap process is designed to be transparent and efficient:

1.  **Wrapper Execution**: You run `./anvilw build`.
2.  **Download Anvil**: The wrapper checks for a cached Anvil executable. If it's not found, it downloads the correct version for your OS from GitHub Releases.
3.  **Compile Build Script**: The downloaded `anvil` executable compiles your project's `build.cpp` using a bundled C++ driver. This creates a temporary `runner` executable in your `.anvil` directory.
4.  **Execute Build Logic**: The `runner` is executed. It contains your project's specific build configuration.
5.  **Download Ninja**: The `runner` checks for `ninja`. If it's not present, it's downloaded automatically.
6.  **Generate Ninja Files**: Your build logic generates a `build.ninja` file.
7.  **Run Ninja**: The `ninja` executable is called, which performs the final, high-performance compilation of your project.

## Getting Started: Using Anvil in Your Project

To start using Anvil for your own C++ project, follow these steps.

### 1. Add the Anvil Wrapper

Copy the following files into your project's root directory:

*   `anvilw` (for Linux/macOS)
*   `anvilw.bat` (for Windows)
*   `wrapper/anvil-wrapper.properties`

These files can be found in the Anvil repository.

### 2. Create a `build.cpp`

Create a `build.cpp` file in your project's root. This is where you will define how your project is built.

Here is a basic example for a simple "hello world" application:

```cpp
// build.cpp
#include "anvil/api.hpp"

// This function is the entry point for your build script.
void configure(anvil::Project& project) {
    project.name = "MyAwesomeApp";

    // Define an executable target
    project.add_executable("my_app", [](anvil::CppApplication& app) {
        // Defaults provided by Anvil:
        // - Standard: C++20
        // - Includes: "src" directory (if exists)
        // - Sources: "src/main/main.cpp" (if exists)
        
        // You can override defaults or add more configuration here:
        // app.standard = anvil::CppStandard::CPP_17;
        // app.set_compiler(anvil::CompilerId::GCC);
    });
    
    // Define a test target
    project.add_test("my_tests", [](anvil::CppApplication& app) {
        // Defaults provided by Anvil:
        // - Standard: C++20
        // - Includes: "src" directory
        // - Sources: Recursively adds all .cpp files in "src/test"
        // - Test Runner: Automatically generates a main() entry point if src/test/test_runner.cpp is missing
    });
}
```

### 3. Build Your Project

Now, you can build your project by running the wrapper:

*   **Linux/macOS:**
    ```bash
    chmod +x ./anvilw
    ./anvilw build
    ```

*   **Windows:**
    ```cmd
    anvilw build
    ```

The final executable will be placed in the `bin/` directory.

### 4. Run the Project

To build and immediately run your application:

```bash
./anvilw run [args...]
```

Any arguments passed after `run` will be forwarded to your application.

### 5. Clean the Build

To remove build artifacts and temporary files:

```bash
./anvilw clean
```

### Example Project

For a complete working example, check out the **[Anvil Demo Project](https://github.com/YvanCywan/anvil_demo)**.

## Building Anvil (for Development)

If you want to contribute to Anvil itself, you'll need to build the project from the source.

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/YvanCywan/anvil.git
    cd anvil
    ```

2.  **Run the build:**
    Anvil builds itself using the same wrapper script.
    ```bash
    ./anvilw build
    ```

This will produce the `anvil` executable in the `bin/` directory.

## Configuration

### `build.cpp` API

You configure your build by calling methods on the `project` and `app` objects.

#### Default Behaviors
Anvil provides sensible defaults to reduce boilerplate:
*   **Executables**:
    *   Sets C++ Standard to `CPP_20`.
    *   Adds `src` to include paths.
    *   Adds `src/main/main.cpp` to sources if it exists.
    *   Links statically on Windows.
*   **Tests**:
    *   Sets C++ Standard to `CPP_20`.
    *   Adds `src` to include paths.
    *   Recursively adds all `.cpp` files in `src/test` to sources.
    *   **Automatic Test Runner**: If `src/test/test_runner.cpp` is missing or empty, Anvil automatically generates a default test runner with a `main` function. To use a custom runner, simply create `src/test/test_runner.cpp` with your own implementation.

#### Overriding Defaults
The configuration lambda passed to `add_executable` or `add_test` runs *after* defaults are applied, allowing you to override them.

```cpp
project.add_executable("my_app", [](anvil::CppApplication& app) {
    // Override C++ Standard
    app.standard = anvil::CppStandard::CPP_17;

    // Override Compiler
    app.set_compiler(anvil::CompilerId::GCC); // Options: Clang, GCC, MSVC

    // Add additional sources
    app.add_source("src/utils.cpp");

    // Add include directories
    app.add_include("include");
    
    // Add preprocessor definitions
    app.add_define("DEBUG_MODE");
    
    // Add linker flags
    app.add_link_flag("-lpthread");
});
```

### Environment Variables

*   `ANVIL_SCRIPT_COMPILER`: Set the compiler used to bootstrap the `build.cpp` script.
    *   Example: `export ANVIL_SCRIPT_COMPILER=gcc`

## The Anvil Wrapper (`anvilw`)

The wrapper is a simple script that ensures the correct version of Anvil is available. Its behavior is configured by `wrapper/anvil-wrapper.properties`:

*   `anvilVersion`: The version of Anvil to use. Can be a specific version (e.g., `0.1.53`) or `latest`.
*   `repoUrl`: The GitHub repository URL to download releases from.
