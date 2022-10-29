cmake_minimum_required(VERSION 3.14)
project(game)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# SDL
FetchContent_Declare(
    sdl
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG release-2.24.1)
FetchContent_MakeAvailable(sdl)

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
file (GLOB SAI_CORE_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/core/*)
file (GLOB SAI_DEBUG_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/debug/*)
file (GLOB SAI_ECS_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/ecs/*)
file (GLOB SAI_GRAPHICS_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/graphics/*)
file (GLOB SAI_JOB_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/job/*)
file (GLOB SAI_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/*)
file (GLOB SAI_TASK_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/task/*)
file (GLOB SAI_THREADING_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/threading/*)
file (GLOB SAI_VIDEO_SOURCES ${PROJECT_SOURCE_DIR}/src/sai/video/*)
file (GLOB T9_SOURCES ${PROJECT_SOURCE_DIR}/src/t9/*)
source_group(game FILES ${GAME_SOURCES})
source_group(game/debug FILES ${GAME_DEBUG_SOURCES})
source_group(game/debug/window FILES ${GAME_DEBUG_WINDOW_SOURCES})
source_group(sai FILES ${SAI_SOURCES})
source_group(sai/core FILES ${SAI_CORE_SOURCES})
source_group(sai/debug FILES ${SAI_DEBUG_SOURCES})
source_group(sai/ecs FILES ${SAI_ECS_SOURCES})
source_group(sai/graphics FILES ${SAI_GRAPHICS_SOURCES})
source_group(sai/job FILES ${SAI_JOB_SOURCES})
source_group(sai/task FILES ${SAI_TASK_SOURCES})
source_group(sai/threading FILES ${SAI_THREADING_SOURCES})
source_group(sai/video FILES ${SAI_VIDEO_SOURCES})
source_group(t9 FILES ${T9_SOURCES})

add_executable(${PROJECT_NAME} WIN32
    ${IMGUI_SOURCES} ${IMGUI_BACKENDS}
    ${SOURCES})
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-W -Wall>)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:DEBUG>>:_CRTDBG_MAP_ALLOC>)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR}/backends)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)

if (MSVC)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>
        VERBATIM)
endif()