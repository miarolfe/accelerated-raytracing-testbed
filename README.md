# ART (Accelerated Raytracing Testbed)

ART is a ray tracing testbed designed to benchmark and compare different spatial acceleration structures used in ray tracing.

## Key features

- [x] Path tracing renderer
- [x] Uniform grid acceleration structure
- [x] Hierarchical uniform grid acceleration structure
- [x] Bounding volume hierarchy (BVH) acceleration structure
- [x] Basic time-based performance benchmarking

## Future work

- [ ] Octree acceleration structure
- [ ] BSP tree acceleration structure
- [ ] k-d tree acceleration structure
- [ ] Comprehensive performance benchmarking
- [ ] Wide range of varied test scenarios

## System requirements

Linux is the only actively supported platform at present. This is a C++17 project; it is untested with older C++ standards and may not compile.

### Dependencies

All dependencies are bundled in the `external/` directory:
- **Catch2** - Header-only C++ testing framework
- **stb_image / stb_image_write** - Single-header image I/O libraries

## Building

### Linux

```bash
./build.sh              # Debug build (default)
./build.sh debug        # Debug build (explicit)
./build.sh release      # Optimized release build
./build.sh test         # Build and run test suite
```

### Output locations

- `bin/Debug/ART` - Debug executable with symbols
- `bin/Release/ART` - Optimized release executable
- `bin/Test/ART` - Test suite executable

## Quick start

```bash
git clone https://github.com/miarolfe/accelerated-raytracing-testbed
cd accelerated-raytracing-testbed
./build.sh release
./bin/Release/ART
```

## Testing

### Running tests

```bash
./build.sh test         # Build test configuration
./bin/Test/ART          # Run test suite
```

### Continuous integration

GitHub Actions automatically runs tests on every push and pull request to the `main` branch. Tests execute on `ubuntu-latest` (Linux environment). Check the Actions tab for build logs and test results.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Copyright © 2026 Mia Rolfe. All rights reserved.

### Third-party licenses

- **Catch2** - Boost Software License 1.0 (see `external/Catch2/`)
- **stb libraries** - Public domain / MIT (see headers in `external/stb/`)

## Acknowledgments

- **Catch2** - [C++ testing framework](https://github.com/catchorg/Catch2)
- **stb libraries** - [Public domain image single-header libraries (stb_image, stb_image_write)](https://github.com/nothings/stb)
- This project, and my interest in ray tracing, were heavily inspired by the ["Ray Tracing in One Weekend" series](https://raytracing.github.io/). Check them out!

---
