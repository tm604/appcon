cmake_minimum_required(VERSION 2.8)
project(appcon)
include (CTest)
enable_testing()

option(USE_CLANG "build application with clang" OFF)
if(USE_CLANG)
	set(CMAKE_CXX_COMPILER "/usr/bin/clang++-3.6")
endif()

set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)

include(set_cxx_norm.cmake)
set_cxx_norm(${CXX_NORM_CXX11})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Wconversion")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()
if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

include_directories(include/)
include_directories(deps/)
if(win32)
else()
	add_definitions(-DBOOST_LOG_DYN_LINK)
endif()
set(Boost_USE_MULTITHREADED ON)
find_package(
    Boost
    COMPONENTS
		log_setup
        chrono
        date_time
        filesystem
        system
        log
        locale
        program_options
    REQUIRED
)
find_package(Threads REQUIRED)

install(
	DIRECTORY include/
	DESTINATION include
)

add_subdirectory(lib)
add_subdirectory(tests)

