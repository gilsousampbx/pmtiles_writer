cmake_minimum_required(VERSION 3.15)
project(PMTilesBundler CXX)

include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG master
)
FetchContent_MakeAvailable(json)

#add_subdirectory(${PROJECT_SOURCE_DIR}/external/gzip-hpp)

add_executable(${PROJECT_NAME} main.cpp)

#target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/external/gzip-hpp/include)
target_include_directories(${PROJECT_NAME} PUBLIC "includes/")

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
#target_link_libraries(${PROJECT_NAME} PRIVATE gzip-hpp)