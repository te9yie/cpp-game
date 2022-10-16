cmake_minimum_required(VERSION 3.14)
project(game)

set(CMAKE_CXX_STANDARD 17)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

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
    GIT_REPOSITORY https://github.com/ocornut/imgui)
FetchContent_MakeAvailable(imgui)

file(GLOB IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/*.h
    ${imgui_SOURCE_DIR}/*.cpp)
file(GLOB IMGUI_BACKENDS
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl*.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl*.cpp)
source_group(imgui FILES ${IMGUI_SOURCES} ${IMGUI_BACKENDS})

file(GLOB_RECURSE T9_SOURCES
	${PROJECT_SOURCE_DIR}/src/t9/*.h
	${PROJECT_SOURCE_DIR}/src/t9/*.cpp)
source_group(t9 FILES ${T9_SOURCES})

file(GLOB_RECURSE SAI_SOURCES
	${PROJECT_SOURCE_DIR}/src/sai/*.h
	${PROJECT_SOURCE_DIR}/src/sai/*.cpp)
source_group(sai FILES ${SAI_SOURCES})

file(GLOB_RECURSE GAME_SOURCES
	${PROJECT_SOURCE_DIR}/src/game/*.h
	${PROJECT_SOURCE_DIR}/src/game/*.cpp)
source_group(game FILES ${GAME_SOURCES})

file(GLOB SOURCES
	${PROJECT_SOURCE_DIR}/src/*.h
	${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} WIN32
    ${IMGUI_SOURCES} ${IMGUI_BACKENDS}
    ${T9_SOURCES}
    ${SAI_SOURCES}
    ${GAME_SOURCES}
    ${SOURCES})
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-W -Wall>)
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