cmake_minimum_required(VERSION 3.20)

project(llm-game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED CONFIG)
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Fetch Dear ImGui
include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.92.5
)

FetchContent_MakeAvailable(imgui)

# Build ImGui as a static library
add_library(imgui_lib STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(imgui_lib PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_lib PUBLIC SDL3::SDL3)

add_executable(llm-game
  src/llm-game.cpp
  src/aipfg/http-client.cpp
  src/aipfg/chat-client.cpp
)

target_include_directories(llm-game PRIVATE src ${CURL_INCLUDE_DIRS})
target_link_libraries(llm-game PRIVATE
  imgui_lib
  SDL3::SDL3
  ${CURL_LIBRARIES}
  nlohmann_json::nlohmann_json
)
target_compile_definitions(llm-game PRIVATE
  IMGUI_FONT_DIR="${imgui_SOURCE_DIR}/misc/fonts/"
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT llm-game)
