# Contributing to Poly

## Prerequisites

- CMake 3.14+
- C++20 compiler (Clang 14+, GCC 12+, or MSVC 2022)
- Ninja (recommended)
- pre-commit (`pip install pre-commit`)

## Development workflow

1. Fork the repository
2. Create a feature branch from `main`
3. Make your changes
4. Push and open a pull request

### Setup

```bash
git clone https://github.com/YOUR_USERNAME/poly.git
cd poly
pre-commit install -t pre-push
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

## Code style

Code formatting is enforced by clang-format. The `.clang-format` config is in the
repo root. The pre-push hook runs formatting checks automatically.

Format manually:

```bash
clang-format -i --style=file path/to/file.cpp
```

## Real-time safety

Code in `process()`, `renderRange()`, and any audio-thread path must not:

- Allocate heap memory (`new`, `malloc`, `std::vector::push_back`)
- Acquire locks (`std::mutex`, `std::lock_guard`)
- Throw exceptions
- Perform I/O (`std::cout`, file operations)

Pre-allocate in `initialize()`. Only clear or reset in `setActive()`.

The `scripts/check-realtime-safety.sh` script scans for violations.

## MSVC portability

Always include standard headers explicitly. GCC and Clang provide `std::sort`,
`std::min`, `std::max` transitively, but MSVC does not:

```cpp
#include <algorithm>  // std::sort, std::min, std::max
#include <utility>    // std::move, std::swap
```

## Testing

All changes must pass the test suite. The engine must build and pass tests
without the VST3 SDK:

```bash
cmake -S . -B build-engine -G Ninja -DPOLY_ENGINE_ONLY=ON
cmake --build build-engine
ctest --test-dir build-engine --output-on-failure
```

## Pull request requirements

- Describe what changed and why
- All CI checks must pass (build, tests, formatting, RT safety)
- Keep changes focused -- one concern per PR
