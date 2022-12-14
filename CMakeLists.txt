cmake_minimum_required(VERSION 3.14)
project(game)

include(FetchContent)

# SDL
FetchContent_Declare(
    SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG release-2.24.1)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL)

# SDL_image
FetchContent_Declare(
    SDL_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image
    GIT_TAG release-2.6.2)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL_image)

# imgui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.88)
FetchContent_MakeAvailable(imgui)

file(GLOB IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/*.h
    ${imgui_SOURCE_DIR}/*.cpp)
file(GLOB IMGUI_BACKENDS
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl*.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl*.cpp)
source_group(imgui FILES ${IMGUI_SOURCES} ${IMGUI_BACKENDS})

file(GLOB_RECURSE SOURCES
	${PROJECT_SOURCE_DIR}/src/*.h
	${PROJECT_SOURCE_DIR}/src/*.cpp)

# filter
file (GLOB GAME_DEBUG_SOURCES ${PROJECT_SOURCE_DIR}/src/game/debug/*)
file (GLOB GAME_DEBUG_WINDOW_SOURCES ${PROJECT_SOURCE_DIR}/src/game/debug/window/*)
file (GLOB GAME_SOURCES ${PROJECT_SOURCE_DIR}/src/game/*)
file (GLOB SAI_ASSET_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/asset/*)
file (GLOB SAI_CORE_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/core/*)
file (GLOB SAI_DEBUG_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/debug/*)
file (GLOB SAI_ECS_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/ecs/*)
file (GLOB SAI_GRAPHICS_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/graphics/*)
file (GLOB SAI_INPUT_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/input/*)
file (GLOB SAI_JOB_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/job/*)
file (GLOB SAI_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/*)
file (GLOB SAI_SYNC_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/sync/*)
file (GLOB SAI_TASK_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/task/*)
file (GLOB SAI_THREADING_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/threading/*)
file (GLOB SAI_VIDEO_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/video/*)
file (GLOB T9_SOURCES ${PROJECT_SOURCE_DIR}/src/t9/*)
source_group(game FILES ${GAME_SOURCES})
source_group(game/debug FILES ${GAME_DEBUG_SOURCES})
source_group(game/debug/window FILES ${GAME_DEBUG_WINDOW_SOURCES})
source_group(sai FILES ${SAI_SOURCES})
source_group(sai/asset FILES ${SAI_ASSET_SOURCES})
source_group(sai/core FILES ${SAI_CORE_SOURCES})
source_group(sai/debug FILES ${SAI_DEBUG_SOURCES})
source_group(sai/ecs FILES ${SAI_ECS_SOURCES})
source_group(sai/graphics FILES ${SAI_GRAPHICS_SOURCES})
source_group(sai/input FILES ${SAI_INPUT_SOURCES})
source_group(sai/job FILES ${SAI_JOB_SOURCES})
source_group(sai/sync FILES ${SAI_SYNC_SOURCES})
source_group(sai/task FILES ${SAI_TASK_SOURCES})
source_group(sai/threading FILES ${SAI_THREADING_SOURCES})
source_group(sai/video FILES ${SAI_VIDEO_SOURCES})
source_group(t9 FILES ${T9_SOURCES})

add_executable(${PROJECT_NAME} WIN32
    ${IMGUI_SOURCES} ${IMGUI_BACKENDS}
    ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-W -Wall>)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:DEBUG>>:_CRTDBG_MAP_ALLOC>)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR}/backends)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL_image_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_image::SDL2_image-static SDL2::SDL2-static SDL2::SDL2main)