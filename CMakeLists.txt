cmake_minimum_required(VERSION 3.10)
project(moravor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Engine sources
file(GLOB ENGINE_SRC engine/*.cpp)
file(GLOB GAME_SRC game/*.cpp)

add_executable(moravor
    main.cpp
    render.cpp
    player.cpp
    level.cpp
    ${ENGINE_SRC} ${GAME_SRC}
)

# Copy assets directory to build directory after build
add_custom_command(TARGET moravor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:moravor>/assets)

target_link_libraries(moravor PRIVATE SDL2_image::SDL2_image)

# SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
# Try find_package, but fallback to pkg-config if necessary
find_package(SDL2_ttf)
if (SDL2_TTF_FOUND)
    target_include_directories(moravor PRIVATE ${SDL2_TTF_INCLUDE_DIRS})
    target_link_libraries(moravor PRIVATE ${SDL2_TTF_LIBRARIES})
else()
    message(STATUS "SDL2_ttf not found by CMake, using pkg-config fallback.")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    target_include_directories(moravor PRIVATE ${SDL2_TTF_INCLUDE_DIRS})
    target_link_libraries(moravor PRIVATE ${SDL2_TTF_LIBRARIES})
    # If still not found, try linking directly
    if(NOT SDL2_TTF_LIBRARIES)
        target_link_libraries(moravor PRIVATE -lSDL2_ttf)
    endif()
endif()

# stb_image (header only)
target_include_directories(moravor PRIVATE third_party/stb)

# pugixml (header only or static lib)
target_include_directories(moravor PRIVATE third_party/pugixml)

# Add more libraries as needed

# Assets (placeholder for asset copying)
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
