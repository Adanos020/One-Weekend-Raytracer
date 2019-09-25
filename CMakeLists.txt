cmake_minimum_required(VERSION 3.1...3.13)

project(One-Weekend-Raytracer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("include")
file(GLOB_RECURSE SOURCES
    "src/*.cpp" "src/**/*.cpp"
    "include/*.hpp" "include/**/*.hpp")

add_executable(One-Weekend-Raytracer ${SOURCES})
target_link_libraries(One-Weekend-Raytracer PUBLIC pthread)