cmake_minimum_required(VERSION 3.16)
project(PRAHAAR LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Fetch SFML automatically (no manual install needed) ---
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
    GIT_SHALLOW ON
)
# Use static SFML runtime linking on Windows to avoid missing-DLL issues
set(SFML_STATIC_LIBRARIES FALSE CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SFML)

# --- Gather source files ---
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(PRAHAAR ${SOURCES})

target_include_directories(PRAHAAR PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(PRAHAAR PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# --- Copy assets folder next to the built executable ---
add_custom_command(TARGET PRAHAAR POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:PRAHAAR>/assets"
)

# --- Copy SFML DLLs next to the executable on Windows ---
if(WIN32)
    add_custom_command(TARGET PRAHAAR POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:PRAHAAR> $<TARGET_FILE_DIR:PRAHAAR>
        COMMAND_EXPAND_LISTS
    )
endif()
