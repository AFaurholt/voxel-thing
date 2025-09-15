cmake_minimum_required(VERSION 3.10.0)
# Enable vcpkg manifest mode
set(VCPKG_MANIFEST_MODE ON)

# Set the vcpkg toolchain file if not already set
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    else()
        message(FATAL_ERROR "VCPKG_ROOT environment variable is not set")
    endif()
endif()

# Add vcpkg installed directory to CMAKE_PREFIX_PATH
if(DEFINED VCPKG_INSTALLED_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
endif()

project(hello-world-voxel VERSION 0.1.0 LANGUAGES C CXX)

# Include FetchContent
include(FetchContent)

# Declare gif.h dependency
FetchContent_Declare(
    gif_h
    GIT_REPOSITORY https://github.com/charlietangora/gif-h.git
    GIT_TAG master
)

# Make gif.h available
FetchContent_MakeAvailable(gif_h)

# Find required packages
find_package(SDL3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${gif_h_SOURCE_DIR}
)

# Add executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    GLEW::GLEW
    glm::glm
)

# Add gif.h include directory
target_include_directories(${PROJECT_NAME} PRIVATE ${gif_h_SOURCE_DIR})

include(CTest)
enable_testing()

