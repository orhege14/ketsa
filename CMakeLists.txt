cmake_minimum_required(VERSION 3.20)

project(Ketsa
    VERSION 0.3.0
    DESCRIPTION "A modern general-purpose programming language"
    LANGUAGES CXX
)

# ============================================================
# C++ Standard
# ============================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Build Options
# ============================================================
option(KETSA_BUILD_TESTS "Build tests" ON)
option(KETSA_VM "Enable bytecode VM" ON)
option(KETSA_JIT "Enable JIT compilation (experimental)" ON)
option(KETSA_LSP "Build language server" OFF)
option(KETSA_STATIC "Static linking" OFF)

# ============================================================
# Source Files
# ============================================================
file(GLOB_RECURSE KETSA_SOURCES
    "src/*.cpp"
)

# ============================================================
# Main Executable
# ============================================================
add_executable(ketsa ${KETSA_SOURCES})

target_include_directories(ketsa PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(ketsa PRIVATE
    KETSA_VERSION="${PROJECT_VERSION}"
    $<$<CONFIG:Debug>:KETSA_DEBUG>
)

# ============================================================
# Compiler Flags
# ============================================================
if(MSVC)
    target_compile_options(ketsa PRIVATE
        /W4
        /utf-8
        /Z7
        /permissive-
        $<$<CONFIG:Release>:/O2 /GS->
    )
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)
else()
    target_compile_options(ketsa PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wno-sign-compare
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
        $<$<CONFIG:Debug>:-g -O0>
    )
endif()

# ============================================================
# Linker Flags
# ============================================================
if(KETSA_STATIC)
    if(MSVC)
        target_link_options(ketsa PRIVATE /MT)
    else()
        target_link_options(ketsa PRIVATE -static)
    endif()
endif()

# ============================================================
# Install
# ============================================================
include(GNUInstallDirs)
install(TARGETS ketsa
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ============================================================
# Post-build: Copy to project root
# ============================================================
if(WIN32)
    add_custom_command(TARGET ketsa POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ketsa>
            ${CMAKE_SOURCE_DIR}/ketsa.exe
        COMMENT "Copying ketsa executable to project root"
    )
else()
    add_custom_command(TARGET ketsa POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ketsa>
            ${CMAKE_SOURCE_DIR}/ketsa
        COMMENT "Copying ketsa executable to project root"
    )
endif()

# ============================================================
# Print Build Info
# ============================================================
message(STATUS "Ketsa v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Bytecode VM: ${KETSA_VM}")
message(STATUS "JIT backend: ${KETSA_JIT}")
