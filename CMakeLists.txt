cmake_minimum_required(VERSION 3.25)
project(PlanetLanding)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(nclgl)

add_executable(PlanetLanding main.cpp Renderer.cpp)
target_include_directories(PlanetLanding PUBLIC .)
target_link_libraries(PlanetLanding PUBLIC nclgl)
