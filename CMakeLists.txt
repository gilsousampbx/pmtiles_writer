cmake_minimum_required(VERSION 3.15)
project(PMTilesBundler CXX)

include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG master
)
FetchContent_MakeAvailable(json)

# Fetch zlib
FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        v1.2.11  # Specify the tag or commit hash you want to use
)
FetchContent_MakeAvailable(zlib)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC "includes/")

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE zlib)
