# Scrollmore

This repository contains two standalone terminal pager programs: `more` and `scroller`.  
They are grouped in this repository because they serve similar purposes, but each is independent and can be used separately.

## `more`

A modern implementation of the classic pager utility.  
Displays text files, piped input, or command-line strings one page at a time.

**Features:**

- Page through text with Enter (page down), Down Arrow (line down), and 'q' or Esc to quit.
- Supports input from files, direct text arguments, or piped data.

**Usage:**

```sh
# View a file
more file.txt

# View a string
more "Line 1: This is some text to page through.
Line 2: You can add multiple lines.
Line 3: Each line will be displayed in the pager."

# Pipe input
cat file.txt | more
```

## `scroller`

An interactive, scrollable pager inspired by programs like `less`, but with a modern interface.  
Unlike `more`, which only pages forward and offers limited navigation, `scroller` allows full vertical scrolling, visual feedback via a scrollbar, and a modern interface for smoother navigation.

**Features:**

- Scroll with Arrow keys, Page Up/Down, Home/End, 'q' or Esc to quit.
- Visual scrollbar and controls.
- Adapts to terminal resizing.
- Supports input from files, direct text arguments, or piped data.

**Usage:**

```sh
# View a file
scroller file.txt

# View a string
scroller "Line 1: This is some text to page through.
Line 2: You can add multiple lines.
Line 3: Each line will be displayed in the pager."

# Pipe input
cat file.txt | scroller
```

## Building

To be able to build `more` and `scroller`, the following libraries are required:

- [`ftxui`](https://github.com/ArthurSonzogni/ftxui)
- [`cxxopts`](https://github.com/jarro2783/cxxopts)

The easiest way to build `more` and `scroller` is using [`vcpkg`](https://github.com/microsoft/vcpkg) for dependencies and the included `CMakePresets.json` for convenient configuration in Visual Studio Code.

1. **Install dependencies via vcpkg:**

    ```sh
    vcpkg install ftxui cxxopts
    ```

2. **Configure and build:**
    - Open the project in VSCode.
    - With the [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools), select the configure preset (`MSVC x64`).
    - Select the active build preset: choose between `Debug` (for debugging, with symbols and no optimizations) or `Release` (for optimized executables).
    - Click "Build" (`ALL_BUILD`) in the CMake toolbar to build both programs.
    - The executables will be placed in `/bin/Debug/` or `/bin/Release/`.

3. **Run or debug:**
    - Use the "Run" or "Debug" buttons in the CMake toolbar to execute or debug the programs directly from VSCode.

### Manual building

If you prefer to build manually or use another editor/IDE, you can run these commands from the project root:

```sh
# Configure (Debug build)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Debug

# Or for Release build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

And if you prefer not to use `vcpkg` either, you can manually install **ftxui** and **cxxopts** and adapt the CMake files (or use another build system/C++ package manager) to link them appropriately. See the documentation for [ftxui](https://github.com/ArthurSonzogni/ftxui) and [cxxopts](https://github.com/jarro2783/cxxopts) for details.

## License

This project is licensed under [CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/).
