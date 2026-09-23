cmake_minimum_required(VERSION 3.25)
project(DebuggingWizard VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build Type" FORCE)
endif()

option(DW_SANITIZE       "Build with AddressSantinizer + UNSan" OFF)
option(DW_WERROR         "Treat warning as error"               OFF)
option(DW_BUILD_LAB      "Build lab experiment"                  ON)
option(DW_BUILD_TESTS    "Build test"                            ON)

add_library(dw_options INTERFACE)
target_include_directories(dw_options INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_compile_options(dw_options INTERFACE -Wall -Wextra -Wpedantic -Wshadow)

add_subdirectory(src)
add_subdirectory(bench)

if(DW_ERROR)
    target_compile_options(dw_options INTERFACE -Werror)
endif()

if(DW_SANITIZE) 
    target_compile_options(dw_options INTERFACE 
    -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(dw_options INTERFACE 
    -fsanitize=address,undefined)
endif()

if(DW_BUILD_LAB)
    add_subdirectory(lab)
endif()

if(DW_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()