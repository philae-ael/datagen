cmake_minimum_required(VERSION 3.31 FATAL_ERROR)
project(
    Datagen
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

add_executable(datagen src/datagen.cpp
    src/core/base.cpp
    src/core/arena.cpp
    src/core/string.cpp
)
target_compile_features(datagen PRIVATE cxx_std_23)
target_include_directories(datagen PRIVATE src)

if (WIN32)
    target_sources(datagen PRIVATE src/os/windows/memory.cpp)
else()
    target_sources(datagen PRIVATE
    src/os/linux/memory.cpp
    )
endif()

target_compile_options(datagen PUBLIC
    $<$<CONFIG:Debug>:-g3 -fno-omit-frame-pointer -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
    -Wall -Wextra -Wpedantic -Wnon-virtual-dtor -Wformat=2 -Wformat-truncation -Wimplicit-fallthrough -Woverloaded-virtual -Wsign-conversion -Wdouble-promotion -Wshadow
    -Wno-unused-parameter -Wno-unused-variable -Wno-nullability-extension -Wno-c99-designator
)
