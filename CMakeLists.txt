cmake_minimum_required(VERSION 3.28)

project(software_renderer LANGUAGES CXX)

set(${CMAKE_CXX_STANDARD} 17)
set(${CMAKE_CXX_STANDARD_REQUIRED} TRUE)

add_executable(software_renderer EXCLUDE_FROM_ALL
    "src/Entry.cpp"
    "src/Window.cpp"
    "src/Triangle.cpp"
    "src/Utils.cpp"
    "src/Matrix.cpp"
    "src/Model.cpp"    
    "src/WindowBase.h"
    "src/Structs.h"
)