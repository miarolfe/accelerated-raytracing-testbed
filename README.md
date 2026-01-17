# ART (Accelerated Raytracing Testbed)

<img width="1920" height="1080" alt="A 10x10x10 3D grid of spheres spaced out" src="https://github.com/user-attachments/assets/ebd9bc40-8a74-40fc-85b7-01dea756f84c" />

ART is a ray tracing testbed designed to benchmark and compare different spatial acceleration structures used in CPU ray tracing.

## Key features

- [x] Path tracing renderer
- [x] Uniform grid acceleration structure
- [x] Hierarchical uniform grid acceleration structure
- [X] Octree acceleration structure
- [X] BSP tree acceleration structure
- [X] k-d tree acceleration structure
- [x] Bounding volume hierarchy (BVH) acceleration structure
- [x] Basic time-based performance benchmarking

## Future work

- [ ] Comprehensive performance benchmarking
- [ ] Wide range of varied test scenarios

## System requirements

Linux and Windows are actively supported. This is a C++17 project; it is untested with older C++ standards and may not compile.

### Dependencies

Most dependencies are bundled in the `external/` directory:
- **Catch2** - Header-only C++ testing framework
- **imgui** - Immediate mode GUI library (GUI builds only)
- **SDL3** - Cross-platform multimedia library (GUI builds only)
  - Windows: Bundled in `external/SDL3/`
  - Linux: Install via package manager (e.g., `sudo apt install libsdl3-dev` or equivalent)
- **stb_image, stb_image_write** - Single-header image I/O libraries

## Building

### Build configurations

The project supports three build modes:
- **GUI** - Interactive graphical interface using SDL3 and imgui
- **Headless** - Command-line only, no GUI dependencies required
- **Test** - Command-line only, unit test suite

### Linux

```bash
./build.sh                      # Debug Headless build (default)
./build.sh debug                # Debug Headless build
./build.sh debug_gui            # Debug GUI build
./build.sh release              # Release Headless build
./build.sh release_gui          # Release GUI build
./build.sh test                 # Build test suite
```

### Windows
```
./generate_vs2022_solution.bat
```
Then open the generated solution and select the desired configuration:
- `Debug_GUI`
- `Debug_Headless`
- `Release_GUI`
- `Release_Headless`
- `Test`

### Output locations

- `bin/Debug_Headless/ART` - Debug headless executable
- `bin/Debug_GUI/ART` - Debug GUI executable
- `bin/Release_Headless/ART` - Optimized headless executable
- `bin/Release_GUI/ART` - Optimized GUI executable
- `bin/Test/ART` - Test suite executable

### Runtime dependencies (Linux)

On Linux, GUI executables require the SDL3 shared library to be installed. Install it via your package manager before running GUI builds:

- **Ubuntu/Debian**: `sudo apt install libsdl3`
- **Fedora**: `sudo dnf install SDL3`

## Quick start

```bash
git clone https://github.com/miarolfe/accelerated-raytracing-testbed
cd accelerated-raytracing-testbed
./build.sh release      # Linux - use "generate_vs2022_solution.bat" on Windows
./bin/Release_Headless/ART
```

## Testing

### Running tests

```bash
./build.sh test         # Build test configuration
./bin/Test/ART          # Run test suite
```

### Continuous integration

GitHub Actions automatically runs tests on every push and pull request to the `main` branch. Tests execute on `ubuntu-latest` and `windows-latest`. Check the Actions tab for build logs and test results.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

### Third-party licenses

- **Catch2** - Boost Software License 1.0 (see `external/Catch2/`)
- **imgui** - MIT License (see `external/imgui/`)
- **SDL3** - zlib License (see `external/SDL3/`)
- **stb_image, stb_image_write** - Public domain / MIT (see headers in `external/stb/`)

## Acknowledgments

- **Catch2** - [C++ testing framework](https://github.com/catchorg/Catch2)
- **imgui** - [Immediate mode graphical user interface](https://github.com/ocornut/imgui)
- **SDL3** - [Platform abstraction layer](https://github.com/libsdl-org/SDL)
- **stb_image, stb_image_write** - [Public domain image single-header libraries (stb_image, stb_image_write)](https://github.com/nothings/stb)
- This project, and my interest in ray tracing, were heavily inspired by the ["Ray Tracing in One Weekend" series](https://raytracing.github.io/). Check them out!

---
