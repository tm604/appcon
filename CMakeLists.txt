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
set (VERSION_PATCH 6)
set (PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set (PROJECT_VERSION_REVISION 1)

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
IF(WIN32)
else()
	add_definitions(-DBOOST_LOG_DYN_LINK)
endif()
set(Boost_USE_MULTITHREADED ON)
find_package(
    Boost
    COMPONENTS
		log_setup
        log
        chrono
        date_time
        filesystem
		thread
        system
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

include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_NAME "libappcon")
set (CPACK_PACKAGE_VENDOR "")
set (CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set (CPACK_PACKAGE_CONTACT "Tom Molesworth <tom@perlsite.co.uk>")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ application config library")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.debian")
set (CPACK_GENERATOR "TGZ;DEB")
set (CPACK_DEBIAN_PACKAGE_SHLIBDEPS "OFF")
string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)
find_program(DPKG_PROGRAM dpkg)
if(DPKG_PROGRAM)
	execute_process(
		COMMAND ${DPKG_PROGRAM} --print-architecture
		OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${PROJECT_VERSION}-${PROJECT_VERSION_REVISION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")   
else(DPKG_PROGRAM)
	set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${PROJECT_VERSION}-${PROJECT_VERSION_REVISION}_${CMAKE_SYSTEM_NAME}")
endif(DPKG_PROGRAM)
# set (CPACK_DEBIAN_PACKAGE_SHLIBDEPS "ON")
include (CPack)
