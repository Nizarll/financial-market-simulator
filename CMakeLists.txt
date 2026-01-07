cmake_minimum_required(VERSION 3.28)
project(fmsm)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(absl REQUIRED)

add_executable(app
  src/main.cpp
  src/widget.cpp
)

target_compile_options(app PUBLIC -g3)
target_include_directories(app PUBLIC libs/)
target_link_libraries(app
  Qt5::Core
  Qt5::Charts
  Qt5::Widgets
  nlohmann_json::nlohmann_json
  absl::base
)

add_custom_command(
    TARGET app
    POST_BUILD
    COMMAND mv compile_commands.json ..
)

