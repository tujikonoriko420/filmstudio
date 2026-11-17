cmake_minimum_required(VERSION 3.20)
project(FilmStudio CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "GTA SA is 32-bit. Configure with: cmake -A Win32")
endif()

# plugin-sdk (DK22Pac). Set PLUGIN_SDK_DIR env var or -DPLUGIN_SDK_DIR=...
if(NOT DEFINED PLUGIN_SDK_DIR)
  set(PLUGIN_SDK_DIR "$ENV{PLUGIN_SDK_DIR}")
endif()
if(NOT EXISTS "${PLUGIN_SDK_DIR}/plugin_sa")
  message(FATAL_ERROR "PLUGIN_SDK_DIR invalido: '${PLUGIN_SDK_DIR}'")
endif()

if(NOT DEFINED PLUGIN_LIB OR PLUGIN_LIB STREQUAL "")
  set(PLUGIN_LIB "${PLUGIN_SDK_DIR}/output/lib/plugin.lib")
endif()
include(FetchContent)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.91.5)
FetchContent_Declare(minhook GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git GIT_TAG v1.3.3)
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG master)
FetchContent_MakeAvailable(imgui minhook stb)

file(GLOB FS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(FilmStudio SHARED ${FS_SRC}
  ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp)
set_target_properties(FilmStudio PROPERTIES PREFIX "" SUFFIX ".asi")
target_compile_definitions(FilmStudio PRIVATE GTASA PLUGIN_SGV_10US _CRT_SECURE_NO_WARNINGS
  _USING_V110_SDK71_ NOMINMAX _NO_CRT_STDIO_INLINE)
target_include_directories(FilmStudio PRIVATE ${stb_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends
  ${PLUGIN_SDK_DIR}/plugin_sa ${PLUGIN_SDK_DIR}/plugin_sa/game_sa ${PLUGIN_SDK_DIR}/shared
  ${PLUGIN_SDK_DIR}/shared/game ${PLUGIN_SDK_DIR}/shared/extensions)
target_link_libraries(FilmStudio PRIVATE minhook
  "${PLUGIN_LIB}" d3d9 dxguid shell32 winmm)
if(MSVC)
  target_compile_options(FilmStudio PRIVATE /MT /utf-8 /wd4244 /wd4996 /wd4267)
endif()
