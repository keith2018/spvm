# SPVM
Tiny [SPIR-V](https://registry.khronos.org/SPIR-V/) virtual machine (interpreter), can be used for shader debugging.

[![License](https://img.shields.io/badge/license-MIT-green)](./LICENSE)
[![CMake Linux](https://github.com/keith2018/spvm/actions/workflows/cmake_linux.yml/badge.svg)](https://github.com/keith2018/spvm/actions/workflows/cmake_linux.yml)
[![CMake MacOS](https://github.com/keith2018/spvm/actions/workflows/cmake_macos.yml/badge.svg)](https://github.com/keith2018/spvm/actions/workflows/cmake_macos.yml)
[![CMake Windows](https://github.com/keith2018/spvm/actions/workflows/cmake_windows.yml/badge.svg)](https://github.com/keith2018/spvm/actions/workflows/cmake_windows.yml)

![](images/awesomeface.png)

The project is still in progress ...

## Spvm-ShaderToy
Spvm-ShaderToy implements [shader effects](https://www.shadertoy.com/) without GPU.

## Clone
```bash
git clone git@github.com:keith2018/spvm.git
cd spvm
git submodule update --init --recursive
```

## Build
```bash
mkdir build
cmake -B ./build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build --config Release
```

## Test
```bash
cd build
ctest
```

## License
This code is licensed under the MIT License (see [LICENSE](LICENSE)).
