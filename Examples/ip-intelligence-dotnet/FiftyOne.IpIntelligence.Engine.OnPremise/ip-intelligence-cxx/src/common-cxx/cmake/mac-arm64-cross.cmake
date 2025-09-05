mark_as_advanced(CMAKE_TOOLCHAIN_FILE)

# the name of the target operating system
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)

set(triple arm64-apple-darwin-eabi)

# which compilers to use
set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER_TARGET ${triple})
