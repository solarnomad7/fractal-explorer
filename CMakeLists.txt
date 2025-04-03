cmake_minimum_required(VERSION 3.16)
project(MandelbrotVisualizer LANGUAGES CXX)

find_package(Lua 5.4 REQUIRED)
include_directories(/usr/local/include ${LUA_INCLUDE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)

set(SFML_VERSION 2.6.x)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui
	GIT_TAG v1.90.6)
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui SOURCE_DIR IMGUI_DIR)

set(IMGUI_SFML_FIND_SFML OFF)
FetchContent_Declare(imgui-sfml
	GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
	GIT_TAG 2.6.x)
FetchContent_MakeAvailable(imgui-sfml)

add_executable(mandelbrot src/Set.h src/MandelbrotSet.h src/JuliaSet.h src/Set.cpp src/MandelbrotSet.cpp src/JuliaSet.cpp src/main.cpp)
target_link_libraries(mandelbrot PRIVATE sfml-graphics)
target_link_libraries(mandelbrot PUBLIC ImGui-SFML::ImGui-SFML)
target_link_libraries(mandelbrot PUBLIC ${LUA_LIBRARIES})
target_include_directories(mandelbrot PUBLIC ${LUA_INCLUDE_DIR})
find_package(OpenMP)
if (OpenMP_CXX_FOUND)
	target_link_libraries(mandelbrot PUBLIC OpenMP::OpenMP_CXX)
endif()
target_compile_features(mandelbrot PRIVATE cxx_std_17)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/scripts/)

if(WIN32)
    add_custom_command(
        TARGET mandelbrot
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:mandelbrot>
        VERBATIM)
endif()

install(TARGETS mandelbrot)
